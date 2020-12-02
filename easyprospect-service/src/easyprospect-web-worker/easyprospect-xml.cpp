/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <easyprospect-web-worker/easyprospect-xml.h>
#include <stdlib.h>
#include <string.h>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>



#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {

        // ---------------------------------------------------------------------------
        //
        //  Recursively Count up the total number of child Elements under the specified Node.
        //  Process attributes of the node, if any.
        //
        // ---------------------------------------------------------------------------
        static int countChildElements(xercesc::DOMNode* n, bool printOutEncounteredEles)
        {
            xercesc::DOMNode* child;
            int               count = 0;
            if (n)
            {
                if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
                {
                    if (printOutEncounteredEles)
                    {
                        char* name = xercesc::XMLString::transcode(n->getNodeName());
                        std::cout << "----------------------------------------------------------" << std::endl;
                        std::cout << "Encountered Element : " << name << std::endl;

                        xercesc::XMLString::release(&name);

                        if (n->hasAttributes())
                        {
                            // get all the attributes of the node
                            xercesc::DOMNamedNodeMap* pAttributes = n->getAttributes();
                            const XMLSize_t           nSize       = pAttributes->getLength();
                            std::cout << "\tAttributes" << std::endl;
                            std::cout << "\t----------" << std::endl;
                            for (XMLSize_t i = 0; i < nSize; ++i)
                            {
                                xercesc::DOMAttr* pAttributeNode = (xercesc::DOMAttr*)pAttributes->item(i);
                                // get attribute name
                                char* name = xercesc::XMLString::transcode(pAttributeNode->getName());

                                std::cout << "\t" << name << "=";
                                xercesc::XMLString::release(&name);

                                // get attribute type
                                name = xercesc::XMLString::transcode(pAttributeNode->getValue());
                                std::cout << name << std::endl;
                                xercesc::XMLString::release(&name);
                            }
                        }
                    }
                    ++count;
                }
                for (child = n->getFirstChild(); child != 0; child = child->getNextSibling())
                    count += countChildElements(child, printOutEncounteredEles);
            }
            return count;
        }
    } // namespace web_worker
} // namespace service
} // namespace easyprospect