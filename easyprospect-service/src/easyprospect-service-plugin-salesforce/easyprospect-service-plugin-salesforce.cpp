#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
//#include <xercesc/framework/StdOutFormatTarget.hpp>
//#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <strstream>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMLSOutput.hpp>
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMTreeWalker.hpp>
#include <xercesc/dom/DOMDocumentTraversal.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xalanc/XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <xalanc/DOMSupport\XalanDocumentPrefixResolver.hpp>
#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>
#include <xalanc/XalanDOM/XalanNamedNodeMap.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>
#include <xalanc/XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <xalanc/XercesParserLiaison/XercesDOMSupport.hpp>
#include <xalanc/XercesParserLiaison/XercesParserLiaison.hpp>

#include <boost/filesystem/string_file.hpp>
#include <easyprospect-service-plugin-salesforce/easyprospect-service-plugin-salesforce.h>
#include <spdlog/spdlog.h>

#include "easyprospect-data-schema/easyprospect-data-schema.h"

namespace easyprospect
{
namespace service
{
    namespace plugin
    {
        easyprospect_service_plugin_salesforce::
        easyprospect_service_plugin_salesforce(const make_shared_enabler&):
            easyprospect_service_plugin(
                "Service Salesforce Plugin",
                common::plugin::ep_plugin_category::TYPE_SERVICE,
                common::plugin::ep_plugin_type_id::PLUGIN_SRV_SALESFORCE,
                boost::uuids::random_generator()())
        {
            // TODO: kp. Called once per process, so this needs to be moved or flag protected
            xercesc::XMLPlatformUtils::Initialize();
            xalanc::XalanTransformer::initialize();
        }

        std::shared_ptr<easyprospect_service_plugin_salesforce> easyprospect_service_plugin_salesforce::get_instance()
        {
            auto res = create<easyprospect_service_plugin_salesforce>();

            return res;
        }

        void easyprospect_service_plugin_salesforce::register_plugin()
        {
        }

        ep_srv_plugin_sf_context* easyprospect_service_plugin_salesforce::init_context_v8()
        {
            auto res = init_context();

            return res.get();
        }

        std::shared_ptr<ep_srv_plugin_sf_context> easyprospect_service_plugin_salesforce::init_context()
        {
            auto sf = shared_from_this();
            auto res = std::make_shared<ep_srv_plugin_sf_context>(sf);

            contexts_.push_back(res);

            return res;
        }

        std::string easyprospect_service_plugin_salesforce::read_str_v8(
            ep_srv_plugin_sf_context* cxt,
            std::string               path_str)
        {
            auto cxtp = cxt->shared_from_this();

            auto res = read_str(cxtp, std::move(path_str));

            return res;
        }

        std::string easyprospect_service_plugin_salesforce::read_str(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               path_str)
        {
            std::string result;
            boost::filesystem::load_string_file(path_str, result);

            return result;
        }

        void easyprospect_service_plugin_salesforce::write_str_v8(
            ep_srv_plugin_sf_context* cxt,
            std::string               content,
            std::string               path_str)
        {
            auto cxtp = cxt->shared_from_this();

            write_str(cxtp, content, path_str);
        }

        void easyprospect_service_plugin_salesforce::write_str(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               content,
            std::string                               path_str)
        {
            boost::filesystem::save_string_file(path_str, content);
        }

        ep_srv_plugin_sf_doc* easyprospect_service_plugin_salesforce::parse_str_v8(
            ep_srv_plugin_sf_context* cxt,
            std::string               content,
            std::string               schema_path)
        {
            auto cxtp = cxt->shared_from_this();

            auto res = parse_str(cxtp, content, schema_path);

            return res.get();
        }

        std::shared_ptr<ep_srv_plugin_sf_doc> easyprospect_service_plugin_salesforce::parse_str(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               content,
            std::string                               schema_path)
        {
            auto theParser = std::make_unique<xercesc::XercesDOMParser>();
            auto errorHandler = std::make_unique<ep_srv_plugin_sf_dom_error_handler>();
            auto entityResolver = std::make_unique<ep_srv_plugin_sf_entity_resolver>(cxt);

            // Turn on validation and namespace support.
            theParser->setValidationScheme(xercesc::XercesDOMParser::Val_Always);
            theParser->setDoNamespaces(true);
            theParser->setDoSchema(true);
          //  theParser->setValidationSchemaFullChecking(true);
            theParser->setErrorHandler(errorHandler.get());
         //   theParser->setValidationConstraintFatal(true);
            theParser->setEntityResolver(entityResolver.get());

            theParser->cacheGrammarFromParse(true);

            auto schema = boost::filesystem::path{schema_path};
            auto schema_path_obj      = boost::filesystem::canonical(schema);
            std::string schema_str;
            boost::filesystem::load_string_file(schema_path_obj, schema_str);

            xercesc::MemBufInputSource schema_buff((XMLByte*)schema_str.c_str(), schema_str.size(), "catalog.xsd");

            if(theParser->loadGrammar(schema_buff, xercesc::Grammar::SchemaGrammarType, true) == nullptr)
            {
                spdlog::error( "couldn't load schema\n");
            }

            //theParser->setParameter(xercesc::XMLUni::fgXercesDOMHasPSVIInfo, true); // collect schema info
            //theParser->setParameter(xercesc::XMLUni::fgDOMComments, false);         // discard comments
            //theParser->setExternalNoNamespaceSchemaLocation("schema.xsd");

            // Instantiate the DOM parser.
            //static const XMLCh          gLS[]  = {xercesc::chLatin_L, xercesc::chLatin_S, xercesc::chNull};
            //xercesc::DOMImplementation* impl   = xercesc::DOMImplementationRegistry::getDOMImplementation(gLS);
            //xercesc::DOMLSParser*       parser = static_cast<xercesc::DOMImplementationLS*>(impl)->createLSParser(
            //    xercesc::DOMImplementationLS::MODE_SYNCHRONOUS, 0);
            //xercesc::DOMConfiguration* config = parser->getDomConfig();

            // config->setParameter(xercesc::XMLUni::fgDOMNamespaces, doNamespaces);
            // config->setParameter(xercesc::XMLUni::fgXercesSchema, doSchema);
            // config->setParameter(xercesc::XMLUni::fgXercesHandleMultipleImports, true);
            // config->setParameter(xercesc::XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
            // config->setParameter(xercesc::XMLUni::fgDOMDisallowDoctype, disallowDoctype);

            // if (valScheme == xercesc::AbstractDOMParser::Val_Auto)
            //{
            //    config->setParameter(xercesc::XMLUni::fgDOMValidateIfSchema, true);
            //}
            // else if (valScheme == xercesc::AbstractDOMParser::Val_Never)
            //{
            //    config->setParameter(xercesc::XMLUni::fgDOMValidate, false);
            //}
            // else if (valScheme == xercesc::AbstractDOMParser::Val_Always)
            //{
            //    config->setParameter(xercesc::XMLUni::fgDOMValidate, true);
            //}

            //// enable datatype normalization - default is off
            // config->setParameter(xercesc::XMLUni::fgDOMDatatypeNormalization, true);

            //// And create our error handler and install it
            // DOMCountErrorHandler errorHandler;
            // config->setParameter(xercesc::XMLUni::fgDOMErrorHandler, &errorHandler);

            // reset document pool
            //parser->resetDocumentPool();

            std::shared_ptr<ep_srv_plugin_sf_doc> res;

            xercesc::DOMDocument* doc = nullptr;
            {
                xercesc::MemBufInputSource xml_buf(
                    reinterpret_cast<const XMLByte*>(content.c_str()), content.size(), "sf xml (in memory)");
                //xercesc::Wrapper4InputSource source(&xml_buf);

                auto startMillis = xercesc::XMLPlatformUtils::getCurrentMillis();

                theParser->parse(xml_buf);
                if(theParser->getErrorCount()!=0)
                {
                    spdlog::error("Parsing failed.");
                }

                res = std::make_shared<ep_srv_plugin_sf_doc>(cxt, std::move(theParser));

                auto endMillis = xercesc::XMLPlatformUtils::getCurrentMillis();
                auto duration  = endMillis - startMillis;
            }

            return res;
        }

        std::string easyprospect_service_plugin_salesforce::str_doc_v8(
            ep_srv_plugin_sf_context* cxt,
            ep_srv_plugin_sf_doc*     doc)
        {
            auto cxtp = cxt->shared_from_this();
            auto docp = doc->shared_from_this();

            auto res = str_doc(cxtp, docp);

            return res;
        }

        std::string easyprospect_service_plugin_salesforce::str_doc(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::shared_ptr<ep_srv_plugin_sf_doc>     doc)
        {
            std::string res;

            try
            {
                // get a serializer, an instance of DOMLSSerializer
                XMLCh                       tempStr[3] = {xercesc::chLatin_L, xercesc::chLatin_S, xercesc::chNull};
                xercesc::DOMImplementation* impl = xercesc::DOMImplementationRegistry::getDOMImplementation(tempStr);
                xercesc::DOMLSSerializer*   theSerializer = ((xercesc::DOMImplementationLS*)impl)->createLSSerializer();
                xercesc::DOMLSOutput*       theOutputDesc = ((xercesc::DOMImplementationLS*)impl)->createLSOutput();

                // set user specified output encoding
                // theOutputDesc->setEncoding(gOutputEncoding);

                //// plug in user's own filter
                // if (gUseFilter)
                //{
                //    // even we say to show attribute, but the DOMLSSerializer
                //    // will not show attribute nodes to the filter as
                //    // the specs explicitly says that DOMLSSerializer shall
                //    // NOT show attributes to DOMLSSerializerFilter.
                //    //
                //    // so DOMNodeFilter::SHOW_ATTRIBUTE has no effect.
                //    // same DOMNodeFilter::SHOW_DOCUMENT_TYPE, no effect.
                //    //
                //    myFilter = new DOMPrintFilter(
                //        DOMNodeFilter::SHOW_ELEMENT | DOMNodeFilter::SHOW_ATTRIBUTE |
                //        DOMNodeFilter::SHOW_DOCUMENT_TYPE);
                //    theSerializer->setFilter(myFilter);
                //}

                // plug in user's own error handler
                auto                       myErrorHandler   = cxt->get_errors();
                xercesc::DOMConfiguration* serializerConfig = theSerializer->getDomConfig();
                serializerConfig->setParameter(xercesc::XMLUni::fgDOMErrorHandler, myErrorHandler.get());

                // set feature if the serializer supports the feature/mode
                // if (serializerConfig->canSetParameter(xercesc::XMLUni::fgDOMWRTSplitCdataSections,
                // gSplitCdataSections))
                //    serializerConfig->setParameter(xercesc::XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections);

                // if (serializerConfig->canSetParameter(xercesc::XMLUni::fgDOMWRTDiscardDefaultContent,
                // gDiscardDefaultContent))
                //    serializerConfig->setParameter(xercesc::XMLUni::fgDOMWRTDiscardDefaultContent,
                //    gDiscardDefaultContent);

                // if (serializerConfig->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint,
                // gFormatPrettyPrint))
                //    serializerConfig->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint);

                // if (serializerConfig->canSetParameter(xercesc::XMLUni::fgDOMWRTBOM, gWriteBOM))
                //    serializerConfig->setParameter(xercesc::XMLUni::fgDOMWRTBOM, gWriteBOM);

                // if (serializerConfig->canSetParameter(xercesc::XMLUni::fgDOMXMLDeclaration, gXMLDeclaration))
                //    serializerConfig->setParameter(xercesc::XMLUni::fgDOMXMLDeclaration, gXMLDeclaration);

                //
                // Plug in a format target to receive the resultant
                // XML stream from the serializer.
                //
                // StdOutFormatTarget prints the resultant XML stream
                // to stdout once it receives any thing from the serializer.
                //
                xercesc::XMLFormatTarget* myFormTarget;
                // if (goutputfile)
                // myFormTarget = new xercesc::LocalFileFormatTarget(goutputfile);
                // else
                // myFormTarget = new xercesc::StdOutFormatTarget();
                myFormTarget = new xercesc::MemBufFormatTarget();
                theOutputDesc->setByteStream(myFormTarget);

                // get the DOM representation
                // xercesc::DOMDocument* doc = parser->getDocument();
                xercesc::DOMDocument* xml_doc = doc->get_doc();

                //
                // do the serialization through DOMLSSerializer::write();
                //
                // if (gXPathExpression != NULL)
                //{
                //    XMLCh*      xpathStr = XMLString::transcode(gXPathExpression);
                //    DOMElement* root     = xml_doc->getDocumentElement();
                //    try
                //    {
                //        DOMXPathNSResolver* resolver = xml_doc->createNSResolver(root);
                //        DOMXPathResult*     result =
                //            xml_doc->evaluate(xpathStr, root, resolver, DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                //            NULL);

                //        XMLSize_t nLength = result->getSnapshotLength();
                //        for (XMLSize_t i = 0; i < nLength; i++)
                //        {
                //            result->snapshotItem(i);
                //            theSerializer->write(result->getNodeValue(), theOutputDesc);
                //        }

                //        result->release();
                //        resolver->release();
                //    }
                //    catch (const DOMXPathException& e)
                //    {
                //        XERCES_STD_QUALIFIER cerr
                //            << "An error occurred during processing of the XPath expression. Msg is:"
                //            << XERCES_STD_QUALIFIER                         endl
                //            << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
                //        retval = 4;
                //    }
                //    catch (const DOMException& e)
                //    {
                //        XERCES_STD_QUALIFIER cerr
                //            << "An error occurred during processing of the XPath expression. Msg is:"
                //            << XERCES_STD_QUALIFIER                         endl
                //            << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
                //        retval = 4;
                //    }
                //    XMLString::release(&xpathStr);
                //}
                // else
                theSerializer->write(xml_doc, theOutputDesc);

                // auto res_xmlstr =
                // xercesc::XMLString::transcode(((xercesc::MemBufFormatTarget*)myFormTarget)->getRawBuffer());
                auto res_xmlstr = (char*)(((xercesc::MemBufFormatTarget*)myFormTarget)->getRawBuffer());
                res             = res_xmlstr;
                theOutputDesc->release();
                theSerializer->release();

                //
                // Filter, formatTarget and error handler
                // are NOT owned by the serializer.
                //
                delete myFormTarget;
                // delete myErrorHandler;

                // if (gUseFilter)
                //     delete myFilter;
            }
            catch(const xercesc::OutOfMemoryException&)
            {
                spdlog::error("OutOfMemoryException");
                // retval = 5;
            }
            catch(const xercesc::DOMLSException& e)
            {
                auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

                spdlog::error("An error occurred during serialization of the DOM tree. Msg is: %s", tmp_str);

                xercesc::XMLString::release(&tmp_str);
                // retval = 4;
            }
            catch(const xercesc::XMLException& e)
            {
                auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

                spdlog::error("An error occurred during creation of output transcoder. Msg is: %s", tmp_str);

                xercesc::XMLString::release(&tmp_str);

                // retval = 4;
            }

            return res;
        }

        ep_srv_plugin_sf_dom_error_handler* easyprospect_service_plugin_salesforce::get_errors_v8(
            ep_srv_plugin_sf_context* cxt)
        {
            auto cxtp = cxt->shared_from_this();

            auto res = get_errors(cxtp);

            return res.get();
        }

        std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> easyprospect_service_plugin_salesforce::get_errors(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt)
        {
            auto res = cxt->get_errors();

            return res;
        }

        void easyprospect_service_plugin_salesforce::transform_str_v8(
            ep_srv_plugin_sf_context* cxt,
            std::string               in_str,
            std::string               out_str,
            std::string               style_str)
        {
            auto cxtp = cxt->shared_from_this();

            transform_str(cxtp, in_str, out_str, style_str);
        }

        void easyprospect_service_plugin_salesforce::transform_str(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               in_str,
            std::string                               out_str,
            std::string                               style_str)
        {
            //// Call the static initializer for Xerces.
            // xercesc::XMLPlatformUtils::Initialize(
            //    xercesc::XMLUni::fgXercescDefaultLocale,
            //    0,
            //    0,
            //    &memoryManager);

            std::istrstream source(in_str.c_str(), in_str.length());
            std::istrstream style(style_str.c_str(), style_str.length());
            std::ostrstream output;

            xalanc::XalanTransformer::initialize();

            {
                // Create a XalanTransformer.
                xalanc::XalanTransformer theXalanTransformer{};

                // The assumption is that the executable will be run
                // from same directory as the input files.
                auto theResult = theXalanTransformer.transform(source, style, output);

                if(theResult != 0)
                {
                    spdlog::error("transform Error: %s \n", theXalanTransformer.getLastError());
                }
            }

            //// Terminate Xalan...
            // xalanc::XalanTransformer::terminate();

            //// Terminate Xerces...
            // xercesc::XMLPlatformUtils::Terminate();

            //// Clean up the ICU, if it's integrated...
            // xalanc::XalanTransformer::ICUCleanUp();
        }

        void easyprospect_service_plugin_salesforce::sf_catalog_split2(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               doc_str,
            std::string
                                        validate_xpath_templates, // if not empty, object added.  E.g. check for a particular child or sibling
            std::map<sf_item_type, int> element_page_size)
        {
            const auto doc_obj = parse_str(cxt, doc_str, cxt->get_catalog_xsd());

            auto doc = doc_obj->get_doc();

            // xercesc::DOMElement* root              = doc->getDocumentElement();
            try
            {
                doc->normalize();

                // https://codesynthesis.com/~boris/blog/2006/11/28/xerces-c-dom-potholes/

                xercesc::DOMTreeWalker* walker =
                    doc->createTreeWalker(
                    doc->getDocumentElement(), xercesc::DOMNodeFilter::SHOW_ALL, NULL, true);

                std::deque<std::shared_ptr<data::schema::salesforce::ep_sf_object>> objects;

                //for(auto *n = walker->firstChild(); n != nullptr; n = walker->nextSibling())
                for(auto *n = walker->getRoot(); n != nullptr; n = walker->nextNode())
                {
                    switch(n->getNodeType())
                    {
                    case xercesc::DOMNode::TEXT_NODE:
                    case xercesc::DOMNode::CDATA_SECTION_NODE:
                    {
                        xercesc::DOMText* t(static_cast<xercesc::DOMText*>(n));

                        char* str(xercesc::XMLString::transcode(t->getData()));
                        spdlog::debug("  text  = '{}'", str);

                        xercesc::XMLString::release(&str);
                    }
                        break;

                    case xercesc::DOMNode::ELEMENT_NODE:
                    {
                        char* name = xercesc::XMLString::transcode(n->getNodeName());

                        spdlog::debug("element  = '{}'", name);

                        xercesc::XMLString::release(&name);
                    }
                        break;

                    case xercesc::DOMNode::ATTRIBUTE_NODE:
                    case xercesc::DOMNode::ENTITY_REFERENCE_NODE:
                    case xercesc::DOMNode::ENTITY_NODE:
                    case xercesc::DOMNode::PROCESSING_INSTRUCTION_NODE:
                    case xercesc::DOMNode::COMMENT_NODE:
                    case xercesc::DOMNode::DOCUMENT_NODE:
                    case xercesc::DOMNode::DOCUMENT_TYPE_NODE:
                    case xercesc::DOMNode::DOCUMENT_FRAGMENT_NODE:
                    case xercesc::DOMNode::NOTATION_NODE:
                    default: ;
                    }
                }
            }
            catch(...)
            {
            }

            spdlog::debug("split completed");

        }

        void easyprospect_service_plugin_salesforce::sf_catalog_split(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::string                               doc_str,
            std::string                     validate_xpath_templates, // if not empty, object added.  E.g. check for a particular child or sibling
            std::map<sf_item_type, int>     element_page_size)
        {
            const auto doc_obj = parse_str(cxt, doc_str, cxt->get_catalog_xsd());

            auto doc = doc_obj->get_doc();

            // xercesc::DOMElement* root              = doc->getDocumentElement();
            try
            {
                doc->normalize();
                // https://apache.github.io/xalan-c/usagepatterns.html#passing-in-a-xerces-dom-to-a-transformation
                xalanc::XercesParserLiaison theParserLiaison;
                xalanc::XercesDOMSupport    theDOMSupport{theParserLiaison};

                // https://flylib.com/books/en/2.131.1/evaluating_an_xpath_expression.html

                const xalanc::XercesDOMWrapperParsedSource parsedSource(
                    doc, theParserLiaison, theDOMSupport); // xalanc::XalanDOMString(xmlInput.getSystemId())
                auto                                xdoc = parsedSource.getDocument();
                xalanc::XalanDocumentPrefixResolver thePrefixResolver{xdoc};
                auto                                curr_prefix_resolver = std::make_unique<ep_xalan_prefix_resolver>(
                    xalanc::XalanDOMString("http://www.demandware.com/xml/impex/catalog/2006-10-31"), 
                    xdoc);

                auto c1 = xdoc->getFirstChild();
                auto t1 = xalanc::TranscodeToLocalCodePage(c1->getNodeName());
                auto t2 = xalanc::c_str(t1);
                spdlog::debug("{}", t2);

                // OK, let's find the context node...
                xalanc::XPathEvaluator theEvaluator;

                xalanc::NodeRefList header_res;
                theEvaluator.selectNodeList(
                    header_res,
                    theDOMSupport,
                    xdoc,
                    xalanc::XalanDOMString("/epsfcatalog:catalog/epsfcatalog:header").c_str(),
                    *curr_prefix_resolver);

                const xalanc::NodeRefList::size_type theLength = header_res.getLength();

                if(theLength == 0)
                {
                    spdlog::debug("No nodes returned");
                }
                else
                {
                    for(xalanc::NodeRefList::size_type i = 0; i < theLength; ++i)
                    {
                        const auto* const theNode = header_res.item(i);
                        assert(theNode != 0);

                        const auto theNodeType = theNode->getNodeType();

                        const auto theNodeName = theNode->getNodeName();

                        spdlog::debug("{}", xercesc::XMLString::transcode(theNodeName.c_str()));
                    }
                }

                xalanc::NodeRefList product_res;
                theEvaluator.selectNodeList(
                    product_res,
                    theDOMSupport,
                    xdoc,
                    xalanc::XalanDOMString("/catalog/product").c_str(),
                    thePrefixResolver);

                xalanc::NodeRefList category_res;
                theEvaluator.selectNodeList(
                    category_res,
                    theDOMSupport,
                    xdoc,
                    xalanc::XalanDOMString("/catalog/category").c_str(),
                    thePrefixResolver);

                xalanc::NodeRefList category_assignment_res;
                theEvaluator.selectNodeList(
                    category_assignment_res,
                    theDOMSupport,
                    xdoc,
                    xalanc::XalanDOMString("/catalog/category-assignment").c_str(),
                    thePrefixResolver);

                xalanc::NodeRefList recommendation_res;
                theEvaluator.selectNodeList(
                    recommendation_res,
                    theDOMSupport,
                    xdoc,
                    xalanc::XalanDOMString("/catalog/recommendation").c_str(),
                    thePrefixResolver);

                std::vector<xalanc::XalanNode*> result_list;

                XMLSize_t nLength = product_res.getLength();

                if(nLength > 0)
                {
                    for(XMLSize_t i = 0; i < nLength; i++)
                    {
                        auto                   node = product_res.item(i);
                        xalanc::XalanDOMString str;

                        const int theType = node->getNodeType();

                        if(theType == xalanc::XalanNode::COMMENT_NODE ||
                           theType == xalanc::XalanNode::PROCESSING_INSTRUCTION_NODE)
                            str = node->getNodeValue();
                        else if(theType == xalanc::XalanNode::ELEMENT_NODE)
                            str = node->getNodeName();
                        else
                            xalanc::DOMServices::getNodeData(*node, str);

                        result_list.push_back(node);

                        // theSerializer->write(nodes_res.getNodeValue(), theOutputDesc);
                    }
                }



                //    result->release();
                //   resolver->release();
            }
            catch(const xercesc::DOMXPathException& e)
            {
                auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

                spdlog::error("An error occurred during processing of the XPath expression. Msg is: %s", tmp_str);

                xercesc::XMLString::release(&tmp_str);
            }
            catch(const xercesc::DOMException& e)
            {
                auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

                spdlog::error("An error occurred during processing of the XPath expression. Msg is: %s", tmp_str);

                xercesc::XMLString::release(&tmp_str);

                // retval = 4;
            }
            // xercesc::XMLString::release(&xpathStr);
        }

        // Build xerces DOM tree.
        // Remove certain elements that are passed in through a list
        // Add paged results
        void walk_xa_catalog(xalanc::XalanNode*n, std::vector<sf_item_type> replaced_types)
        {
            if(n == nullptr)
            {
                return;
            }

             if(n->getNodeType() == xalanc::XalanNode::ELEMENT_NODE)
            {
                xalanc::CharVectorType name;

                try
                {
                    n->getNodeName().transcode(name);
                    const auto* nstr = xalanc::c_str(name);
                    spdlog::info("node name '%s'", nstr);
                }
                catch(const xalanc::XalanDOMString::TranscodingError&)
                {
                    spdlog::error("transcoding failed");
                }

                 auto attrs = n->getAttributes();

                if(attrs != nullptr)
                 {
                     // get all the attributes of the node
                     const XMLSize_t  nSize       = attrs->getLength();
                    spdlog::info("attribute count %d", nSize);

                    for(auto i=0; i<nSize; i++ )
                    {
                        auto aNode = attrs->item(i);

                        try
                        {
                            aNode->getNodeName().transcode(name);
                            const auto* nstr = xalanc::c_str(name);
                            spdlog::info("node attribute name '%s'", nstr);

                            aNode->getNodeValue().transcode(name);
                            nstr = xalanc::c_str(name);
                            spdlog::info("node attribute value '%s'", nstr);
                        }
                        catch(const xalanc::XalanDOMString::TranscodingError&)
                        {
                            spdlog::error("transcoding failed");
                        }
                    }
                 }

                //for(auto child = n->getFirstChild(); child != nullptr; child = child->getNextSibling())
                //     walk_xa_tree(child);
            }
        }

        //void easyprospect_service_plugin_salesforce::sf_catalog_split(
        //    std::shared_ptr<ep_srv_plugin_sf_context> cxt,
        //    std::string                               doc_str,
        //    std::string                               include_xpath,
        //    std::map<std::string, int>                element_page_size)
        //{
        //    const auto doc_obj = parse_str(cxt, doc_str, cxt->get_catalog_xsd());

        //    auto doc = doc_obj->get_doc();

        //    // xercesc::DOMElement* root              = doc->getDocumentElement();
        //    try
        //    {
        //        doc->normalize();
        //        // https://apache.github.io/xalan-c/usagepatterns.html#passing-in-a-xerces-dom-to-a-transformation
        //        xalanc::XercesParserLiaison theParserLiaison;
        //        xalanc::XercesDOMSupport    theDOMSupport{theParserLiaison};

        //        // https://flylib.com/books/en/2.131.1/evaluating_an_xpath_expression.html

        //        const xalanc::XercesDOMWrapperParsedSource parsedSource(
        //            doc.get(), theParserLiaison, theDOMSupport); // xalanc::XalanDOMString(xmlInput.getSystemId())
        //        auto                                xdoc = parsedSource.getDocument();
        //        xalanc::XalanDocumentPrefixResolver thePrefixResolver{xdoc};

        //        // OK, let's find the context node...
        //        xalanc::XPathEvaluator theEvaluator;

        //        bool page = true;

        //        do
        //        {
        //            // xalanc::XalanNode* const theContextNode = theEvaluator.selectSingleNode(
        //            //    theDOMSupport, xdoc,
        //            //    xalanc::XalanDOMString("/").c_str(),
        //            //    thePrefixResolver);

        //            // Parse the document...
        //            // xalanc::XalanDocument* const theDocument = theParserLiaison.parseXMLStream(parsedSource);

        //            // assert(theDocument != 0);
        //            // const xalanc::XObjectPtr theResult(theEvaluator.evaluate(
        //            //    theDOMSupport,
        //            //    theContextNode,
        //            //    xalanc::XalanDOMString(include_xpath.c_str()).c_str(),
        //            //    thePrefixResolver));

        //            xalanc::NodeRefList nodes_res;
        //            auto                res_list = theEvaluator.selectNodeList(
        //                nodes_res,
        //                theDOMSupport,
        //                xdoc,
        //                xalanc::XalanDOMString(include_xpath.c_str()).c_str(),
        //                thePrefixResolver);

        //            std::vector<xalanc::XalanNode*> result_list;

        //            XMLSize_t nLength = nodes_res.getLength();

        //            if(nLength <= 0)
        //            {
        //                page = false;
        //                continue;
        //            }

        //            for(XMLSize_t i = 0; i < nLength; i++)
        //            {
        //                auto                   node = nodes_res.item(i);
        //                xalanc::XalanDOMString str;

        //                const int theType = node->getNodeType();

        //                if(theType == xalanc::XalanNode::COMMENT_NODE ||
        //                   theType == xalanc::XalanNode::PROCESSING_INSTRUCTION_NODE)
        //                    str = node->getNodeValue();
        //                else if(theType == xalanc::XalanNode::ELEMENT_NODE)
        //                    str = node->getNodeName();
        //                else
        //                    xalanc::DOMServices::getNodeData(*node, str);

        //                result_list.push_back(node);

        //                // theSerializer->write(nodes_res.getNodeValue(), theOutputDesc);
        //            }
        //        } while(page);

        //        //    result->release();
        //        //   resolver->release();
        //    }
        //    catch(const xercesc::DOMXPathException& e)
        //    {
        //        auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

        //        spdlog::error("An error occurred during processing of the XPath expression. Msg is: %s", tmp_str);

        //        xercesc::XMLString::release(&tmp_str);
        //    }
        //    catch(const xercesc::DOMException& e)
        //    {
        //        auto tmp_str = xercesc::XMLString::transcode(e.getMessage());

        //        spdlog::error("An error occurred during processing of the XPath expression. Msg is: %s", tmp_str);

        //        xercesc::XMLString::release(&tmp_str);

        //        // retval = 4;
        //    }
        //    // xercesc::XMLString::release(&xpathStr);
        //}

        ep_srv_plugin_sf_context::ep_srv_plugin_sf_context(
            std::shared_ptr<easyprospect_service_plugin_salesforce> plugin)
        {
            plugin_ = plugin;
            errors_ = std::make_shared<ep_srv_plugin_sf_dom_error_handler>();
        }

    } // namespace plugin
} // namespace service
} // namespace easyprospect