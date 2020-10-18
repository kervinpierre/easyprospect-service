#include "EPCT_Utils.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::unique_ptr<EpJsResult>
EPCT::EPCT_Utils::ContextSingleUse(easyprospect_v8& ep, std::string script)
{
    int r = 0;
    unsigned int id = 0;
    if ((r = ep.create_context(id)) != Success)
       throw std::logic_error("Context error");

    // Exception thrown on error
    auto res = ep.run_javascript(id, script);

    ep.remove_context(id);

    return res;
}

std::string EPCT::EPCT_Utils::ContextSingleUseToString(easyprospect_v8 &ep, std::string script)
{
    std::string res;

    // Exception thrown on error
    auto resObj = ContextSingleUse(ep, script);

    res = resObj->ToString();

    return res;
}

bool EPCT::EPCT_Utils::ContextSingleUseFromFile(easyprospect_v8 &ep, std::string scriptPath)
{ 
    auto jsCode = readFile(scriptPath);

    auto res = ContextSingleUse(ep, jsCode);

    bool bRes = res->ToBool();

    return bRes;
}

std::string EPCT::EPCT_Utils::Context(easyprospect_v8& ep, unsigned int id, std::string script)
{
    int r = 0;
    std::string res;
    //if ((r = ep.run_javascript(id, script, res)) != Success)
    auto resObj = ep.run_javascript(id, script);

    res = resObj->ToString();

    return res;
}

bool EPCT::EPCT_Utils::ContextFromFile(unsigned int id, std::string scriptPath)
{
    return false;
}

std::vector<EPCT::EPCT_FILE_TEST> EPCT::EPCT_Utils::ProcessJSDir(std::string testDirStr, int testCaseId)
{
    spdlog::info("Running testing in '{}'", testDirStr);

    std::vector<EPCT::EPCT_FILE_TEST> res;

    boost::filesystem::path testDir(testDirStr);
    testDir = boost::filesystem::system_complete(testDir);

    try
    {
        if (boost::filesystem::exists(testDir))
        {
            if (boost::filesystem::is_regular_file(testDir))
            {
                spdlog::trace(str(boost::format("%s size is %d\n")
                    % testDir % boost::filesystem::file_size(testDir)));
            }
            else if (boost::filesystem::is_directory(testDir))
            {
                spdlog::debug(str(boost::format("%s is a directory containing:\n")
                    % testDir));

                std::vector<boost::filesystem::path> pathsList;

                for (auto&& currFile : boost::filesystem::directory_iterator(testDir))
                {
                    pathsList.push_back(currFile.path());
                }

                std::sort(pathsList.begin(), pathsList.end());

                std::regex re = std::regex(EPCT_JS_SOURCE_REGEX);

                for (auto&& x : pathsList)
                {
                    spdlog::trace(str(boost::format("Processing '%s'\n")
                        % x));

                    std::smatch matches;

                    std::string fileName = x.filename().string();

                    if (std::regex_match(fileName, matches, re))
                    {
                        spdlog::debug(str(boost::format("Matched '%s'\n")
                            % x));

                        EPCT::EPCT_FILE_TEST t{ matches[1], matches[2], matches[3], matches[4] };

                        if (testCaseId == 0 || testCaseId == std::stoi(t.testCase))
                        {
                            res.push_back(t);
                        }
                    }
                }
            }
            else
            {
                spdlog::info(str(boost::format("'%s' exists, but is not a regular file or directory\n")
                    % testDir));
            }
        }
        else
        {
            spdlog::error(str(boost::format("'%s' does not exist\n")
                % testDir));
        }
    }
    catch (const boost::filesystem::filesystem_error & ex)
    {
        spdlog::error("{}\n", ex.what());
    }

    return res;
}

std::string EPCT::EPCT_Utils::readFile(std::string path)
{
    if( !boost::filesystem::exists(path))
    {
        auto p = boost::filesystem::current_path();
        auto p2 = boost::filesystem::canonical(p);

        auto msg = fmt::format("'{}' does not exist.\nCWD: {}", path, p2.string());
        spdlog::error( msg );

        throw std::logic_error(msg);
    }

    std::ifstream inFile;
    inFile.open(path);

    std::stringstream strStream;
    strStream << inFile.rdbuf();
    
    return strStream.str();
}
