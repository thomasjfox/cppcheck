/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
#ifndef checkunusedvarH
#define checkunusedvarH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

#include <map>
#include <string>

class Type;
class Scope;
class Variables;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckUnusedVar : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckUnusedVar() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUnusedVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckUnusedVar checkUnusedVar(tokenizer, settings, errorLogger);

        // Coding style checks
        checkUnusedVar.checkStructMemberUsage();
        checkUnusedVar.checkFunctionVariableUsage();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        (void)tokenizer;
        (void)settings;
        (void)errorLogger;
    }

    /** @brief %Check for unused function variables */
    void checkFunctionVariableUsage_iterateScopes(const Scope* const scope, Variables& variables, bool insideLoop);
    void checkVariableUsage(const Scope* const scope, const Token* start, Variables& variables);
    void checkFunctionVariableUsage();

    /** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

private:
    bool isRecordTypeWithoutSideEffects(const Type* type);
    bool isEmptyType(const Type* type);

    // Error messages..
    void unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname);
    void unusedVariableError(const Token *tok, const std::string &varname);
    void allocatedButUnusedVariableError(const Token *tok, const std::string &varname);
    void unreadVariableError(const Token *tok, const std::string &varname);
    void unassignedVariableError(const Token *tok, const std::string &varname);

    // used by multi-file analysis
    static void unusedGlobalVariableError(ErrorLogger * const errorLogger,
                                          const std::string &filename, unsigned int lineNumber,
                                          const std::string &varname);
    static void globalVariableCanBeStaticError(ErrorLogger * const errorLogger,
            const std::string &filename, unsigned int lineNumber,
            const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckUnusedVar c(0, settings, errorLogger);

        // style/warning
        c.unusedVariableError(0, "varname");
        c.allocatedButUnusedVariableError(0, "varname");
        c.unreadVariableError(0, "varname");
        c.unassignedVariableError(0, "varname");
        c.unusedStructMemberError(0, "structname", "variable");
        c.unusedGlobalVariableError(errorLogger, "", 0, "varname");
        c.globalVariableCanBeStaticError(errorLogger, "", 0, "varname");
    }

    /// data structure for inter-file global variable usage analysis
    struct CPPCHECKLIB GlobalVariableUsage {
        std::string varName;
        std::string filename;
        unsigned int lineNumber;

        bool isStatic;
        bool isExtern;
        std::set<std::string> used_in_files;

        GlobalVariableUsage()
            : lineNumber(0)
            , isStatic(false)
            , isExtern(false) {
        }

        GlobalVariableUsage(const std::string &name,
                            const std::string &filename_,
                            unsigned int linenr,
                            bool is_static,
                            bool is_extern)
            : varName(name)
            , filename(filename_)
            , lineNumber(linenr)
            , isStatic(is_static)
            , isExtern(is_extern) {
        }
    };

    /* data for multifile checking */
    struct VarUsageFileInfo : public Check::FileInfo {
        std::vector<CheckUnusedVar::GlobalVariableUsage> globalvar_usage;
    };

    std::map<unsigned int, CheckUnusedVar::GlobalVariableUsage> collectGlobalVariables(const Tokenizer *tokenizer) const;
    static void scan_token_varusage(const Token *start,
                                    const Token *end,
                                    std::map<unsigned int, GlobalVariableUsage> &usage,
                                    const std::string &filename);
    static void usage_add_file(const Token *tok,
                               std::map<unsigned int, GlobalVariableUsage> &usage,
                               const std::string &filename);

    void determineGlobalVariableUsage(std::map<unsigned int, CheckUnusedVar::GlobalVariableUsage> &usage,
                                      const Tokenizer *tokenizer) const;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const;

    /** @brief Analyse all file infos for all TU */
    void analyseWholeProgram(const std::list<Check::FileInfo*> &fileInfo, ErrorLogger &errorLogger);

    static std::string myName() {
        return "UnusedVar";
    }

    std::string classInfo() const {
        return "UnusedVar checks\n"

               // style
               "- unused variable\n"
               "- allocated but unused variable\n"
               "- unred variable\n"
               "- unassigned variable\n"
               "- unused struct member\n";
    }

    std::map<const Type *,bool> isRecordTypeWithoutSideEffectsMap;

    std::map<const Type *,bool> isEmptyTypeMap;

};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedvarH
