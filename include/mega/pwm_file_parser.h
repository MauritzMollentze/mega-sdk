#ifndef INCLUDE_MEGA_PWM_FILE_PARSER_H_
#define INCLUDE_MEGA_PWM_FILE_PARSER_H_

#include "mega/types.h"

namespace mega::pwm::import
{

/**
 * @class PassEntryParseResult
 * @brief A helper struct to store the result from parsing an entry in a file with passwords to
 * import
 *
 */
struct PassEntryParseResult
{
    enum class ErrCode : uint8_t
    {
        OK = 0,
        INVALID_NUM_OF_COLUMN,
    };

    /**
     * @brief An error code associated to a problem that invalidates the parsing of the entry.
     */
    ErrCode mErrCode = ErrCode::OK;

    /**
     * @brief The line number of the entry in the file. This can be useful to report the source of
     * the problems.
     */
    uint32_t mLineNumber{0};

    /**
     * @brief The name that labels the password entry.
     *
     * @note This struct does not force any condition on this member, i.e. empty is a valid value
     */
    std::string mName;

    /**
     * @brief Members for the different fields that can be found in an entry
     */
    std::string mUrl;
    std::string mUserName;
    std::string mPassword;
    std::string mNote;
};

/**
 * @class PassFileParseResult
 * @brief A helper struct to hold a full report of the parsing password file process.
 *
 */
struct PassFileParseResult
{
    enum class ErrCode : uint8_t
    {
        OK = 0,
        NO_VALID_ENTRIES,
        FILE_DOES_NOT_EXIST,
        CANT_OPEN_FILE,
        MISSING_COLUMN,
    };

    /**
     * @brief An error code associated to a problem that invalidates the file parsing as a whole.
     */
    ErrCode mErrCode = ErrCode::OK;

    /**
     * @brief An error messages with additional information useful for logging.
     */
    std::string mErrMsg;

    /**
     * @brief A vector with a PassEntryParseResult object with the parse information for each of the
     * rows found in the file.
     */
    std::vector<PassEntryParseResult> mResults;
};

/**
 * @brief Reads the password entries listed in the input csv file exported from Google's Password
 * application.
 *
 * @param filePath The path to the csv file.
 * @return A container holding all the valid and invalid entries found together, with handy error
 * codes and messages.
 */
PassFileParseResult parseGooglePasswordCSVFile(const std::string& filePath);

}

#endif // INCLUDE_MEGA_PWM_FILE_PARSER_H_
