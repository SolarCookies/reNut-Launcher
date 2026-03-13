#include <iostream>
#include <stdio.h>
#include <chrono>

#include "Windows/Window.h"
#include <thread>
#include <string>
#include <mutex>
#include <vector>
#include <direct.h>
#include "Windows/Fonts.h"
#include "Windows/Patches.h"
#include "Windows/Background.h"
#include "Windows/ChangeLog.h"
#include "Windows/Credits.h"
#include "Utils/INIManager.h"
#include "Utils/FileDownloader.h"
#include "Utils/IsoExtraction.h"
#include "Utils/VersionManager.h"
#include "Utils/GitHubAPI.h"
#include "Utils/LogSystem.h"
#include <filesystem>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <sstream>
#include <atomic>
#include "Windows/SmartStyles.h"

#pragma comment(lib, "shell32.lib")

// Hide console window
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int, char**)
{
	LogStreambuf logBuf(std::cout.rdbuf());
	std::cout.rdbuf(&logBuf);

	INI::EnsureINIFileExists();

	VinceWindow window1(1920, 1080, INI::GetString("Title", "N&B Launcher", "Window").c_str());

	BackgroundManager backgroundManager;
	ChangeLog changeLog;
	Credits credits;
	std::shared_ptr<DownloadProgress> downloadProgress = nullptr;
	std::shared_ptr<IsoExtractionProgress> isoProgress = nullptr;

	std::string latestCommitSha;
	std::string savedCommitSha;
	bool isCheckingForUpdates = false;
	bool updateAvailable = false;

	bool showLogWindow = INI::GetBool("log", "window", "true");

	bool gameIsRunning = false;
	HANDLE gameProcessHandle = nullptr;

	window1.SetupImGuiIO();
	Patches patchesWindow;
	patchesWindow.Init();

	backgroundManager.Initialize();

	changeLog.Initialize(
			INI::GetString("Name", "masterspike52/reNut", "Github"),
			INI::GetString("Branch", "main", "Github"));

	credits.Initialize(INI::GetString("Name", "masterspike52/reNut", "Github"));

	savedCommitSha = VersionManager::LoadSavedVersion();

	if (!isCheckingForUpdates) {
		isCheckingForUpdates = true;
		GitHubAPI::FetchLatestCommitShaAsync(
			INI::GetString("Name", "masterspike52/reNut", "Github"),
			INI::GetString("Branch", "main", "Github"),
			[&](const std::string& sha) {
				latestCommitSha = sha;
				isCheckingForUpdates = false;

				if (!savedCommitSha.empty() && !latestCommitSha.empty() && savedCommitSha != latestCommitSha) {
					updateAvailable = true;
					std::cout << "Update available! Current: " << savedCommitSha << " Latest: " << latestCommitSha << std::endl;
				} else if (savedCommitSha == latestCommitSha) {
					std::cout << "Up to date! Version: " << savedCommitSha << std::endl;
				}
			});
	}
	PushSmartStyle(INI::GetImColor("BaseColor_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("BaseColor_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));

	glfwSwapInterval(1);  // Enables vsync
	using clock = std::chrono::high_resolution_clock;
	auto lastTime = clock::now();
	auto lastUpdateCheck = clock::now();
	const auto updateCheckInterval = std::chrono::minutes(5);
	while (!glfwWindowShouldClose(window1.getWindow()))
	{
		auto currentTime = clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		float deltaTime = elapsed.count();
		lastTime = currentTime;

		backgroundManager.Update(deltaTime);

		if (gameIsRunning && gameProcessHandle != nullptr) {
			DWORD exitCode;
			if (GetExitCodeProcess(gameProcessHandle, &exitCode)) {
				if (exitCode != STILL_ACTIVE) {
					gameIsRunning = false;
					CloseHandle(gameProcessHandle);
					gameProcessHandle = nullptr;
					std::cout << "Game process has ended." << std::endl;
				}
			} else {
				gameIsRunning = false;
				CloseHandle(gameProcessHandle);
				gameProcessHandle = nullptr;
				std::cout << "Lost track of game process, assuming it ended." << std::endl;
			}
		}

		if (currentTime - lastUpdateCheck >= updateCheckInterval && !isCheckingForUpdates) {
			lastUpdateCheck = currentTime;
			isCheckingForUpdates = true;
			GitHubAPI::FetchLatestCommitShaAsync(
				INI::GetString("Name", "masterspike52/reNut", "Github"),
				INI::GetString("Branch", "main", "Github"),
				[&](const std::string& sha) {
					latestCommitSha = sha;
					isCheckingForUpdates = false;

					if (!savedCommitSha.empty() && !latestCommitSha.empty() && savedCommitSha != latestCommitSha) {
						updateAvailable = true;
						std::cout << "Update check: Update available! Current: " << savedCommitSha << " Latest: " << latestCommitSha << std::endl;
					} else if (savedCommitSha == latestCommitSha) {
						updateAvailable = false;
						std::cout << "Update check: Up to date! Version: " << savedCommitSha << std::endl;
					}
				});
		}

		glfwPollEvents();
		if (glfwGetWindowAttrib(window1.getWindow(), GLFW_ICONIFIED) != 0)
		{
			continue;
		}

		window1.NewFrame();

		ImGui::PushFont(BasicTextFont);

		PushSmartStyle(INI::GetImColor("MenuBar_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("MenuBar_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));
		// Main Menu Bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Set Game Folder")) {
					// Open folder dialog
					BROWSEINFOW bi = { 0 };
					bi.lpszTitle = L"Select Game Folder";
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

					LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
					if (pidl != nullptr) {
						wchar_t path[MAX_PATH];
						if (SHGetPathFromIDListW(pidl, path)) {
							// Convert to UTF-8
							int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
							std::string gamePath;
							gamePath.resize(len - 1);
							WideCharToMultiByte(CP_UTF8, 0, path, -1, &gamePath[0], len, NULL, NULL);

							// Ensure path ends with a slash
							if (!gamePath.empty() && gamePath.back() != '\\' && gamePath.back() != '/') {
								gamePath += "/";
							}

							INI::SetString("GamePath", gamePath, "Game");
							std::cout << "Game path set to: " << gamePath << std::endl;
						}
						CoTaskMemFree(pidl);
					}
				}

				if (ImGui::MenuItem("Uninstall")) {
					// Uninstall game files - use dynamic paths
					std::string gameBasePath = INI::GetString("GamePath", "Game/", "Game");
					if (!gameBasePath.empty() && gameBasePath.back() != '\\' && gameBasePath.back() != '/') {
						gameBasePath += "/";
					}

					std::string exeName = INI::GetString("ExeName", "renut.exe", "Game");
					std::string fullGamePath = gameBasePath + "Game/" + exeName;
					std::string gameAssetsDir = gameBasePath + "Game/assets/";

					bool hasRemovedFiles = false;

					// Remove executable
					if (std::filesystem::exists(fullGamePath)) {
						try {
							std::filesystem::remove(fullGamePath);
							hasRemovedFiles = true;
							std::cout << "Removed: " << fullGamePath << std::endl;
						} catch (const std::exception& e) {
							std::cout << "Error removing " << fullGamePath << ": " << e.what() << std::endl;
						}
					}

					// Remove assets directory
					if (std::filesystem::exists(gameAssetsDir)) {
						try {
							std::filesystem::remove_all(gameAssetsDir);
							hasRemovedFiles = true;
							std::cout << "Removed assets directory: " << gameAssetsDir << std::endl;
						} catch (const std::exception& e) {
							std::cout << "Error removing assets directory " << gameAssetsDir << ": " << e.what() << std::endl;
						}
					}

					// Remove version file
					try {
						VersionManager::RemoveVersionFile();
						hasRemovedFiles = true;
					} catch (const std::exception& e) {
						std::cout << "Error removing version file: " << e.what() << std::endl;
					}

					if (hasRemovedFiles) {
						updateAvailable = false;
						std::cout << "Game uninstalled successfully." << std::endl;
					} else {
						std::cout << "No files to uninstall." << std::endl;
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Close")) {
					glfwSetWindowShouldClose(window1.getWindow(), GLFW_TRUE);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		PopSmartStyle();


		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		backgroundManager.Render(viewport);
		ImGui::PopStyleColor(2);

		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);

		PushSmartStyle(INI::GetImColor("Warning_forground", ImColor(1.0f, 0.0f, 0.0f), "Theme"), INI::GetImColor("Warning_background", ImColor(1.0f, 0.0f, 0.0f), "Theme"));
		//Warning
		{
			ImGui::Begin("NotPlayableYetWarning");
			ImGui::Text("Not intended to be in a playable state yet!");
			ImGui::End();
		}
		PopSmartStyle();

		PushSmartStyle(INI::GetImColor("LaunchButton_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("LaunchButton_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));
		//Launch Button
		{
		ImGui::Begin("Launch Button");
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 85.0f);
		ImGui::PopFont();
		ImGui::PushFont(BasicTitleFont);

		// Get dynamic game path
		std::string gameBasePath = INI::GetString("GamePath", "Game/", "Game");
		if (!gameBasePath.empty() && gameBasePath.back() != '\\' && gameBasePath.back() != '/') {
			gameBasePath += "/";
		}

		std::string exeName = INI::GetString("ExeName", "renut.exe", "Game");
		std::string fullGamePath = gameBasePath + "Game/" + exeName;
		bool gameExists = std::filesystem::exists(fullGamePath);

		if (isoProgress && isoProgress->isExtracting.load()) {
			ImGui::Text("Extracting ISO...");

			// (since we can't easily track exact percentage)
			ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(-1.0f, 0.0f), "Processing...");

			if (isoProgress->hasError.load()) {
				ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Error: %s", isoProgress->errorMessage.c_str());
				if (ImGui::Button("Retry", ImVec2(-1.0f, 30.0f))) {
					std::string isoPath = IsoExtraction::OpenIsoFileDialog();
					if (!isoPath.empty()) {
						isoProgress = std::make_shared<IsoExtractionProgress>();
						IsoExtraction::ExtractIsoAsync(isoPath, isoProgress);
					}
				}
			}
		}
		else if (downloadProgress && !downloadProgress->isComplete.load()) {
      ImGui::Text(INI::GetString("downloading_text","Downloading renut.exe...", "game").c_str());

			float progress = downloadProgress->GetPercentage() / 100.0f;
			char progressText[64];

			if (downloadProgress->totalBytes.load() > 0) {
				sprintf_s(progressText, "%.1f%% (%.2f MB / %.2f MB)", 
					downloadProgress->GetPercentage(),
					downloadProgress->bytesDownloaded.load() / (1024.0f * 1024.0f),
					downloadProgress->totalBytes.load() / (1024.0f * 1024.0f));
			} else {
				sprintf_s(progressText, "Connecting...");
				progress = 0.0f;
			}

			ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), progressText);

			if (downloadProgress->hasError.load()) {
				ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Error: %s", downloadProgress->errorMessage.c_str());
				if (ImGui::Button("Retry", ImVec2(-1.0f, 30.0f))) {
					downloadProgress = std::make_shared<DownloadProgress>();
					FileDownloader::DownloadFileAsync(
            INI::GetString("DownloadFile","https://github.com/masterspike52/reNut/raw/main/out/build/win-amd64-relwithdebinfo/renut.exe","github").c_str(),
						fullGamePath,
						downloadProgress,
						[&]() {
							if (!latestCommitSha.empty()) {
								VersionManager::SaveCurrentVersion(latestCommitSha);
								savedCommitSha = latestCommitSha;
								updateAvailable = false;
								std::cout << "Version saved after retry download: " << latestCommitSha << std::endl;
							}
						});
				}
			}
		} else {
			if (downloadProgress && downloadProgress->isComplete.load() && !downloadProgress->hasError.load()) {
				downloadProgress.reset();
				gameExists = std::filesystem::exists(fullGamePath); // sanity check
			}

			if (isoProgress && isoProgress->isComplete.load() && !isoProgress->hasError.load()) {
				isoProgress.reset();
			}

			// Get dynamic game paths
			std::string gameBasePath = INI::GetString("GamePath", "Game/", "Game");
			if (!gameBasePath.empty() && gameBasePath.back() != '\\' && gameBasePath.back() != '/') {
				gameBasePath += "/";
			}

			std::string defaultXexPath = gameBasePath + "Game/assets/default.xex";
			std::string gameAssetsDir = gameBasePath + "Game/assets/";
			bool assetsExist = false;

			if (std::filesystem::exists(gameAssetsDir) && std::filesystem::is_directory(gameAssetsDir)) {
				int fileCount = 0;
				try {
					for (const auto& entry : std::filesystem::directory_iterator(gameAssetsDir)) {
						if (entry.is_regular_file()) {
							fileCount++;
						}
					}

					assetsExist = fileCount > 0 && std::filesystem::exists(defaultXexPath);

				} catch (const std::exception& e) {
					std::cout << "Error checking assets directory: " << e.what() << std::endl;
					assetsExist = false;
				}
			}

      if (!assetsExist && std::filesystem::exists(defaultXexPath) && !(isoProgress && isoProgress->isExtracting.load())) {
        assetsExist = true;
      }

			const char* buttonText;
			bool buttonEnabled = true;

			if (gameIsRunning) {
				buttonText = "Running";
				buttonEnabled = false;
      } else if (assetsExist && gameExists && !updateAvailable) {
        buttonText = "Launch Game";
      } else if (!assetsExist) {
				buttonText = "Select Game Iso";
			} else if (!gameExists) {
				buttonText = "Download";
			} else if (updateAvailable) {
				buttonText = "Update Available";
			} else {
				buttonText = "Launch Game";
			}

			if (!buttonEnabled) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
			}

			if (ImGui::Button(buttonText, ImVec2(-1.0f, 75.0f)) && buttonEnabled) {
				if (!assetsExist) {
					std::string isoPath = IsoExtraction::OpenIsoFileDialog();
					if (!isoPath.empty()) {
						isoProgress = std::make_shared<IsoExtractionProgress>();
						IsoExtraction::ExtractIsoAsync(isoPath, isoProgress);
					}
				} else if (!gameExists || updateAvailable) {
					if (updateAvailable && gameExists) {
						try {
							std::filesystem::remove(fullGamePath);
							VersionManager::RemoveVersionFile();
							gameExists = false;
						} catch (const std::exception& e) {
							std::cout << "Error when removing old exe: " << e.what() << std::endl;
						}
					}

					downloadProgress = std::make_shared<DownloadProgress>();
					FileDownloader::DownloadFileAsync(
						INI::GetString("DownloadFile", "https://github.com/masterspike52/reNut/raw/main/out/build/win-amd64-relwithdebinfo/renut.exe", "github").c_str(),
						fullGamePath,
						downloadProgress,
						[&]() {
							if (!latestCommitSha.empty()) {
								VersionManager::SaveCurrentVersion(latestCommitSha);
								savedCommitSha = latestCommitSha;
								updateAvailable = false;
							}
						});
				} else {
					std::string patches = patchesWindow.GetPatchLaunchConfig();
					std::string exeFileName = INI::GetString("ExeName", "renut.exe", "Game");

					// Get dynamic game path for launching
					std::string gameBasePath = INI::GetString("GamePath", "Game/", "Game");
					if (!gameBasePath.empty() && gameBasePath.back() != '\\' && gameBasePath.back() != '/') {
						gameBasePath += "/";
					}
					std::string gameDir = gameBasePath + "Game";

					std::thread([patches, exeFileName, gameDir, &gameIsRunning, &gameProcessHandle]() {
						std::string command = "\"" + exeFileName + "\"" + patches;

						STARTUPINFOA si;
						PROCESS_INFORMATION pi;
						ZeroMemory(&si, sizeof(si));
						si.cb = sizeof(si);
						ZeroMemory(&pi, sizeof(pi));

						std::string currentDir;
						char buffer[MAX_PATH];
						if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
							currentDir = buffer;
						}
						SetCurrentDirectoryA(gameDir.c_str());

						if (CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
							gameIsRunning = true;
							gameProcessHandle = pi.hProcess;
							CloseHandle(pi.hThread);
							std::cout << "Game launched successfully." << std::endl;
						} else {
							std::cout << "Failed to launch game. Error: " << GetLastError() << std::endl;
						}

						if (!currentDir.empty()) {
							SetCurrentDirectoryA(currentDir.c_str());
						}
					}).detach();
				}
			}

			if (!buttonEnabled) {
				ImGui::PopStyleColor(3);
			}

		}
		ImGui::End();
    ImGui::PopFont();
    ImGui::PushFont(BasicTextFont);
	}
		PopSmartStyle();
		
		PushSmartStyle(INI::GetImColor("changeLog_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("changeLog_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));
		changeLog.Render();
		PopSmartStyle();

		PushSmartStyle(INI::GetImColor("credits_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("credits_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));
		credits.Render();
		PopSmartStyle();

		//saveListPage.Render();

		PushSmartStyle(INI::GetImColor("patches_forground", ImColor(0.02f, 0.10f, 0.20f), "Theme"), INI::GetImColor("patches_background", ImColor(0.20f, 0.80f, 0.40f), "Theme"));
		if (patchesWindow.HasPatches()) {
			ImGui::Begin("Patches and Mods");
			patchesWindow.Render();
			ImGui::End();
		}
		PopSmartStyle();

		ImGui::PopFont();

		// Render log window
		//LogSystem::GetInstance().Render(&showLogWindow); //Removed due to it being ugly and im to lazy to make it better atm

		ImGui::Render();

		window1.EndFrame();

	}
	PopSmartStyle();

  // Half assed cleanup (To be fair i never actually cared about doing this)
	if (gameProcessHandle != nullptr) {
		CloseHandle(gameProcessHandle);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window1.getWindow());
	glfwTerminate();

	return 0;
}
