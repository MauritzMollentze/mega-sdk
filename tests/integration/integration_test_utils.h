/**
 * @file
 * @brief File aimed to contain utilities for integration tests where objects from megaapi.h are
 * required. For examples, a function to wait for a sync state to change.
 *
 * These utilities extend the ones defined in the more general level for the tests
 * (sdk_test_utils.h) so the namespace is extended (sdk_test).
 */

#ifndef INCLUDE_INTEGRATION_INTEGRATION_TEST_UTILS_H_
#define INCLUDE_INTEGRATION_INTEGRATION_TEST_UTILS_H_

#include "gmock/gmock.h"
#include "mega/types.h"
#include "megaapi.h"
#include "sdk_test_utils.h"

#include <filesystem>
#include <memory>
#include <optional>

namespace sdk_test
{
using namespace std::chrono_literals;
static constexpr auto MAX_TIMEOUT = 3min; // Timeout for operations in this file
static constexpr auto MAX_TIMEOUT_IN_SECS =
    MAX_TIMEOUT * 60; // Timeout for operations in this file in secs
#ifdef ENABLE_SYNC

/**
 * @brief Waits for the sync state to be set to a given value and with a given error during a
 * certain amount of time (30 seconds).
 *
 * @param megaApi The api from where to get the sync object
 * @param remoteNode The root remote node the sync is tracking
 * @param runState The expected run state to match
 * @param err The expected error code to match
 * @return std::unique_ptr<MegaSync> If the sync matches de expected state within that time, the
 * function returns the sync object. Otherwise nullptr.
 */
std::unique_ptr<::mega::MegaSync> waitForSyncState(::mega::MegaApi* megaApi,
                                                   ::mega::MegaNode* remoteNode,
                                                   ::mega::MegaSync::SyncRunningState runState,
                                                   ::mega::MegaSync::Error err);

/**
 * @brief Overloaded implementation where the sync is obtained by the backup id instead of by the
 * remote root node.
 */
std::unique_ptr<::mega::MegaSync> waitForSyncState(::mega::MegaApi* megaApi,
                                                   ::mega::handle backupID,
                                                   ::mega::MegaSync::SyncRunningState runState,
                                                   ::mega::MegaSync::Error err);

/**
 * @brief Synchronously start a TWO_WAY sync between the given local path and the remote node with
 * the given handle.
 *
 * It will also wait until the new sync is in RUNSTATE_RUNNING state.
 *
 * @param megaApi The api to request the sync creation
 * @param localRootPath The local root to sync
 * @param remoteRootHandle The handle of the remote node to sync
 * @return The backupId of the new sync.
 */
::mega::handle syncFolder(::mega::MegaApi* megaApi,
                          const std::string& localRootPath,
                          const ::mega::MegaHandle remoteRootHandle);

/**
 * @brief Synchronously start a BACKUP sync with the given local path
 *
 * It will also wait until the new sync is in RUNSTATE_RUNNING state.
 *
 * @param megaApi The api to request the sync creation
 * @param localRootPath The local path to backup
 * @param backupName The name of the backup. By default, it will use the name of the root directory
 * @return The backupId of the new sync.
 */
::mega::handle backupFolder(::mega::MegaApi* megaApi,
                            const std::string& localRootPath,
                            const std::string& backupName = "");

/**
 * @brief Synchronously removes the sync with the given backupId
 *
 * @return true if the operation succeed, false otherwise
 */
bool removeSync(::mega::MegaApi* megaApi, const ::mega::handle backupID);

/**
 * @brief Synchronously change the running state of the sync with the given backupId
 *
 * @return true if the operation succeed, false otherwise
 */
bool setSyncRunState(::mega::MegaApi* megaApi,
                     const ::mega::handle backupID,
                     const ::mega::MegaSync::SyncRunningState state);

/**
 * @brief Synchronously resume the sync with the given backupId
 *
 * @return true if the operation succeed, false otherwise
 */
bool resumeSync(::mega::MegaApi* megaApi, const ::mega::handle backupID);

/**
 * @brief Synchronously suspend the sync with the given backupId
 *
 * @return true if the operation succeed, false otherwise
 */
bool suspendSync(::mega::MegaApi* megaApi, const ::mega::handle backupID);

/**
 * @brief Synchronously disable the sync with the given backupId
 *
 * @return true if the operation succeed, false otherwise
 */
bool disableSync(::mega::MegaApi* megaApi, const ::mega::handle backupID);

/**
 * @brief Get a vector with all the reported stalls.
 */
std::vector<std::unique_ptr<::mega::MegaSyncStall>> getStalls(::mega::MegaApi* megaApi);

#endif

/**
 * @brief Get a vector with the names of the nodes that are children of the node with the given
 * handle.
 *
 * If any of the operations to get the nodes fails, a nullopt is returned.
 */
std::optional<std::vector<std::string>>
    getCloudFirstChildrenNames(::mega::MegaApi* megaApi, const ::mega::MegaHandle nodeHandle);

/**
 * @brief Get the map resulting from invoking
 * MegaApi::getUserAttribute(MegaApi::USER_ATTR_DEVICE_NAMES) and put it in the output parameter.
 *
 * This function `ASSERT`s on the result from the internal request and also it asserts false if the
 * timeout is exceeded while waiting for it.
 */
void getDeviceNames(::mega::MegaApi* megaApi, std::unique_ptr<::mega::MegaStringMap>& output);

/**
 * @brief Ensures there is at least one device visible to the given megaApi instance. This is
 * required to enable the creation of backup syncs for instance.
 *
 * If there are no devices, a new one is created with the name "Jenkins " + timestamp
 */
void ensureAccountDeviceName(::mega::MegaApi* megaApi);

/**
 * @brief Returns true if value matches the given matcher and also executes EXPECT_THAT
 */
template<typename T, typename MatcherT>
bool checkAndExpectThat(const T& value, const MatcherT& matcher)
{
    bool matched = ::testing::Value(value, matcher);
    EXPECT_THAT(value, matcher);
    return matched;
}

/**
 * @brief createLocalFolder creates a folder in local filesystem
 *
 * @param p input filesystem path
 * @return a filesystem path with the path of created folder, or an empty filesystem path if it
 * couldn't create it
 */
fs::path createLocalFolder(const fs::path& p);

/**
 * @brief Downloads a file from MEGA
 * @see MegaApi::startDownload for more details
 * @return a Numeric errCode corresponding to MegaError received at
 * MegaTransferListener::onTransferFinish, or nullopt if onTransferFinish is never called.
 */
std::optional<int> downloadFile(::mega::MegaApi* megaApi,
                                ::mega::MegaNode* node,
                                const std::filesystem::path& fsPath,
                                const char* customName,
                                const char* appData,
                                const bool startFirst,
                                ::mega::MegaCancelToken* cancelToken,
                                const int collisionCheck,
                                const int collisionResolution,
                                const bool undelete,
                                const std::chrono::seconds timeoutInSecs = MAX_TIMEOUT_IN_SECS);

/**
 * @brief Uploads the file in the given path to the given parentNode.
 */
std::unique_ptr<::mega::MegaNode> uploadFile(::mega::MegaApi* megaApi,
                                             const std::filesystem::path& localPath,
                                             ::mega::MegaNode* parentNode = nullptr);

/**
 * @brief Overloaded version to upload a temp file.
 */
std::unique_ptr<::mega::MegaNode> uploadFile(::mega::MegaApi* megaApi,
                                             LocalTempFile&& file,
                                             ::mega::MegaNode* parentNode = nullptr);

::mega::handle createPasswordNode(::mega::MegaApi* megaApi,
                                  const std::string& name,
                                  const ::mega::MegaNode::PasswordNodeData* data,
                                  const ::mega::handle parentNodeHandle);
}

#endif // INCLUDE_INTEGRATION_INTEGRATION_TEST_UTILS_H_
