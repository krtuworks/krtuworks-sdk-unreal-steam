// Steam for the Krtuworks SDK (the Unity package's SteamworksPlatformServices): the web API ticket for the
// `krtuworks-id` identity (the Steam guest and every session's licence check) and the achievements
// and user stats Steam holds, and the Steam Cloud files the SDK mirrors the mirrored save slots to. Online Subsystem Steam starts Steam and pumps its callbacks; the Krtuworks subsystem asks for the
// services when the game instance starts and gets them only when Steam is running.
#include "CoreMinimal.h"
#include "KrtuworksCore/Steam.h"
#include "KrtuworksCore/Types.h"
#include "KrtuworksSubsystem.h"
#include "Modules/ModuleManager.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace
{
	constexpr std::chrono::milliseconds LeaderboardWait{10000};

	/** The two call results of one own-row read, and what they said. The mutex guards everything below it. */
	struct FLeaderboardRead
	{
		void OnFound(LeaderboardFindResult_t* Result, bool bIOFailure)
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			bBoardFound = !bIOFailure && Result->m_bLeaderboardFound != 0;
			Board = Result->m_hSteamLeaderboard;
			bFound = true;
			Arrived.notify_all();
		}

		void OnDownloaded(LeaderboardScoresDownloaded_t* Result, bool bIOFailure)
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			bDownloadOk = !bIOFailure;
			Entries = Result->m_hSteamLeaderboardEntries;
			EntryCount = Result->m_cEntryCount;
			bDownloaded = true;
			Arrived.notify_all();
		}

		bool Wait(const bool& bDone)
		{
			std::unique_lock<std::mutex> Lock(Mutex);
			return Arrived.wait_for(Lock, LeaderboardWait, [&bDone] { return bDone; });
		}

		/** An answer that comes after this must not reach a read nobody waits for. */
		void Cancel()
		{
			FindCall.Cancel();
			DownloadCall.Cancel();
		}

		std::mutex Mutex;
		std::condition_variable Arrived;
		bool bFound = false;
		bool bBoardFound = false;
		SteamLeaderboard_t Board = 0;
		bool bDownloaded = false;
		bool bDownloadOk = false;
		SteamLeaderboardEntries_t Entries = 0;
		int EntryCount = 0;
		CCallResult<FLeaderboardRead, LeaderboardFindResult_t> FindCall;
		CCallResult<FLeaderboardRead, LeaderboardScoresDownloaded_t> DownloadCall;
	};

	/** Steamworks behind krtu::SteamBackend: the calls, nothing else. */
	class FSteamBackend : public krtu::SteamBackend
	{
	public:
		bool userAvailable() override
		{
			return SteamUser() != nullptr;
		}

		bool statsAvailable() override
		{
			return SteamUserStats() != nullptr;
		}

		std::uint32_t requestWebApiTicket(const std::string& Identity) override
		{
			return SteamUser()->GetAuthTicketForWebApi(Identity.c_str());
		}

		void cancelAuthTicket(std::uint32_t Handle) override
		{
			if (SteamUser() != nullptr)
			{
				SteamUser()->CancelAuthTicket(Handle);
			}
		}

		std::uint32_t numAchievements() override
		{
			return SteamUserStats()->GetNumAchievements();
		}

		std::optional<std::string> achievementName(std::uint32_t Index) override
		{
			const char* Name = SteamUserStats()->GetAchievementName(Index);
			return Name != nullptr ? std::optional<std::string>(Name) : std::nullopt;
		}

		bool getAchievement(const std::string& Name, bool& bAchieved) override
		{
			return SteamUserStats()->GetAchievement(Name.c_str(), &bAchieved);
		}

		bool setAchievement(const std::string& Name) override
		{
			return SteamUserStats()->SetAchievement(Name.c_str());
		}

		bool clearAchievement(const std::string& Name) override
		{
			return SteamUserStats()->ClearAchievement(Name.c_str());
		}

		bool getStatInt(const std::string& Name, std::int32_t& Value) override
		{
			return SteamUserStats()->GetStat(Name.c_str(), &Value);
		}

		bool getStatFloat(const std::string& Name, float& Value) override
		{
			return SteamUserStats()->GetStat(Name.c_str(), &Value);
		}

		bool setStatInt(const std::string& Name, std::int32_t Value) override
		{
			return SteamUserStats()->SetStat(Name.c_str(), Value);
		}

		bool setStatFloat(const std::string& Name, float Value) override
		{
			return SteamUserStats()->SetStat(Name.c_str(), Value);
		}

		void storeStats() override
		{
			SteamUserStats()->StoreStats();
		}

		/**
		 * The local user's own row, read and never written: FindLeaderboard, then DownloadLeaderboardEntriesForUsers for the
		 * local user only. This runs on an SDK thread while the game thread pumps Steam's callbacks (the ticket works the
		 * same way); each call result is waited for up to LeaderboardWait, and a late one is dropped.
		 */
		krtu::PlatformScoreRead ownLeaderboardEntry(const std::string& Name, std::int32_t& Score) override
		{
			if (SteamUserStats() == nullptr || SteamUser() == nullptr)
			{
				return krtu::PlatformScoreRead::failed;
			}
			const std::shared_ptr<FLeaderboardRead> Read = std::make_shared<FLeaderboardRead>();
			Read->FindCall.Set(SteamUserStats()->FindLeaderboard(Name.c_str()), Read.get(), &FLeaderboardRead::OnFound);
			if (!Read->Wait(Read->bFound) || !Read->bBoardFound)
			{
				Read->Cancel();
				return krtu::PlatformScoreRead::failed;
			}
			CSteamID Me = SteamUser()->GetSteamID();
			Read->DownloadCall.Set(SteamUserStats()->DownloadLeaderboardEntriesForUsers(Read->Board, &Me, 1), Read.get(), &FLeaderboardRead::OnDownloaded);
			if (!Read->Wait(Read->bDownloaded) || !Read->bDownloadOk)
			{
				Read->Cancel();
				return krtu::PlatformScoreRead::failed;
			}
			if (Read->EntryCount == 0)
			{
				return krtu::PlatformScoreRead::absent;
			}
			LeaderboardEntry_t Entry;
			if (!SteamUserStats()->GetDownloadedLeaderboardEntry(Read->Entries, 0, &Entry, nullptr, 0))
			{
				return krtu::PlatformScoreRead::failed;
			}
			Score = Entry.m_nScore;
			return krtu::PlatformScoreRead::found;
		}

		bool cloudAvailable() override
		{
			return SteamRemoteStorage() != nullptr;
		}

		bool cloudEnabled() override
		{
			return SteamRemoteStorage()->IsCloudEnabledForAccount() && SteamRemoteStorage()->IsCloudEnabledForApp();
		}

		bool fileExists(const std::string& Name) override
		{
			return SteamRemoteStorage()->FileExists(Name.c_str());
		}

		std::optional<std::string> fileRead(const std::string& Name) override
		{
			const int32 Size = SteamRemoteStorage()->GetFileSize(Name.c_str());
			if (Size < 0)
			{
				return std::nullopt;
			}
			std::string Data(static_cast<std::size_t>(Size), '\0');
			if (SteamRemoteStorage()->FileRead(Name.c_str(), Data.data(), Size) != Size)
			{
				return std::nullopt;
			}
			return Data;
		}

		std::int64_t fileTimestamp(const std::string& Name) override
		{
			return SteamRemoteStorage()->GetFileTimestamp(Name.c_str());
		}

		bool fileWrite(const std::string& Name, const std::string& Data) override
		{
			if (Data.size() > static_cast<std::size_t>(INT32_MAX))
			{
				return false;
			}
			return SteamRemoteStorage()->FileWrite(Name.c_str(), Data.data(), static_cast<int32>(Data.size()));
		}

		bool getQuota(std::uint64_t& Total, std::uint64_t& Available) override
		{
			uint64 SteamTotal = 0;
			uint64 SteamAvailable = 0;
			if (!SteamRemoteStorage()->GetQuota(&SteamTotal, &SteamAvailable))
			{
				return false;
			}
			Total = SteamTotal;
			Available = SteamAvailable;
			return true;
		}

		std::optional<bool> isSubscribedApp(std::uint32_t AppId) override
		{
			ISteamApps* Apps = SteamApps();
			if (Apps == nullptr)
			{
				return std::nullopt;
			}
			return Apps->BIsSubscribedApp(AppId);
		}

		bool setRichPresence(const std::string& Key, const std::string& Value) override
		{
			ISteamFriends* Friends = SteamFriends();
			return Friends != nullptr && Friends->SetRichPresence(Key.c_str(), Value.c_str());
		}

		void clearRichPresence() override
		{
			if (ISteamFriends* Friends = SteamFriends())
			{
				Friends->ClearRichPresence();
			}
		}
	};

	/** The platform services the Krtuworks subsystem asks for: the sync over Steam, plus the ticket callback. */
	class FSteamPlatformServices : public krtu::PlatformServices
	{
	public:
		std::optional<std::string> webApiTicket() override
		{
			return Sync.webApiTicket();
		}

		std::vector<std::string> unlockedAchievements() override
		{
			return Sync.unlockedAchievements();
		}

		void unlockAchievement(const std::string& PlatformId) override
		{
			Sync.unlockAchievement(PlatformId);
		}

		void clearAchievement(const std::string& PlatformId) override
		{
			Sync.clearAchievement(PlatformId);
		}

		krtu::PlatformStats* stats() override
		{
			return &Sync;
		}

		krtu::PlatformCloud* cloud() override
		{
			return &Sync;
		}

		krtu::PlatformPresence* presence() override
		{
			return &Sync;
		}

	private:
		krtu::SteamAchievementSync Sync{std::make_unique<FSteamBackend>()};

		STEAM_CALLBACK(FSteamPlatformServices, OnTicket, GetTicketForWebApiResponse_t);
	};

	void FSteamPlatformServices::OnTicket(GetTicketForWebApiResponse_t* Answer)
	{
		const bool bOk = Answer->m_eResult == k_EResultOK;
		Sync.onTicketAnswer(Answer->m_hAuthTicket, bOk ? Answer->m_rgubTicket : nullptr, Answer->m_cubTicket);
	}
}

class FKrtuworksSteamModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UKrtuworksSubsystem::SetPlatformServicesFactory([]() -> std::shared_ptr<krtu::PlatformServices> {
			// Steam not running (the game started outside it, or Online Subsystem Steam is off): the platform's own sign-in.
			if (!SteamAPI_IsSteamRunning() || SteamUser() == nullptr)
			{
				return nullptr;
			}
			return std::make_shared<FSteamPlatformServices>();
		});
	}

	virtual void ShutdownModule() override
	{
		UKrtuworksSubsystem::SetPlatformServicesFactory(nullptr);
	}
};

IMPLEMENT_MODULE(FKrtuworksSteamModule, KrtuworksSteam)
