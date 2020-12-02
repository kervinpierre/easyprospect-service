#pragma once

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {
        // ---------------------------------------------------------------------------
        //  Simple error handler deriviative to install on parser
        // ---------------------------------------------------------------------------
        class DOMCountErrorHandler : public xercesc::DOMErrorHandler
        {
          public:
            // -----------------------------------------------------------------------
            //  Constructors and Destructor
            // -----------------------------------------------------------------------
            DOMCountErrorHandler();
            ~DOMCountErrorHandler();

            // -----------------------------------------------------------------------
            //  Getter methods
            // -----------------------------------------------------------------------
            bool getSawErrors() const;

            // -----------------------------------------------------------------------
            //  Implementation of the DOM ErrorHandler interface
            // -----------------------------------------------------------------------
            bool handleError(const xercesc::DOMError& domError);
            void resetErrors();

          private:
            // -----------------------------------------------------------------------
            //  Unimplemented constructors and operators
            // -----------------------------------------------------------------------
            DOMCountErrorHandler(const DOMCountErrorHandler&);
            void operator=(const DOMCountErrorHandler&);

            // -----------------------------------------------------------------------
            //  Private data members
            //
            //  fSawErrors
            //      This is set if we get any errors, and is queryable via a getter
            //      method. Its used by the main code to suppress output if there are
            //      errors.
            // -----------------------------------------------------------------------
            bool fSawErrors;
        };

        // ---------------------------------------------------------------------------
        //  This is a simple class that lets us do easy (though not terribly efficient)
        //  trancoding of XMLCh data to local code page for display.
        // ---------------------------------------------------------------------------
        class StrX
        {
          public:
            // -----------------------------------------------------------------------
            //  Constructors and Destructor
            // -----------------------------------------------------------------------
            StrX(const XMLCh* const toTranscode)
            {
                // Call the private transcoding method
                fLocalForm = xercesc::XMLString::transcode(toTranscode);
            }

            ~StrX()
            {
                xercesc::XMLString::release(&fLocalForm);
            }

            // -----------------------------------------------------------------------
            //  Getter methods
            // -----------------------------------------------------------------------
            const char* localForm() const
            {
                return fLocalForm;
            }

          private:
            // -----------------------------------------------------------------------
            //  Private data members
            //
            //  fLocalForm
            //      This is the local code page form of the string.
            // -----------------------------------------------------------------------
            char* fLocalForm;
        };

        inline std::ostream& operator<<(std::ostream& target, const StrX& toDump)
        {
            target << toDump.localForm();
            return target;
        }

        inline bool DOMCountErrorHandler::getSawErrors() const
        {
            return fSawErrors;
        }

        int ep_xml_init(int argC, char* argV[]);

    } // namespace web_worker
} // namespace service
} // namespace easyprospect