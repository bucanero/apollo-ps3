#include <sysutil/sysutil.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

#include "menu.h"
#include "saves.h"

#define DEFAULT_USERNAME "Saved User"


void set_xml_owner(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;
	char *value;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;

		LOG("node type: Element, name: %s\n", cur_node->name);
		if (xmlStrcasecmp(cur_node->name, BAD_CAST "console") == 0)
		{
			value = (char*) xmlGetProp(cur_node, BAD_CAST "idps");
			if (value)
			{
				sscanf(value, "%lx %lx\n", &apollo_config.idps[0], &apollo_config.idps[1]);
				LOG("xml idps=%s", value);
			}

			value = (char*) xmlGetProp(cur_node, BAD_CAST "psid");
			if (value)
			{
				sscanf(value, "%lx %lx\n", &apollo_config.psid[0], &apollo_config.psid[1]);
				LOG("xml psid=%s", value);
			}
		}

		if (xmlStrcasecmp(cur_node->name, BAD_CAST "user") == 0)
		{
			value = (char*) xmlGetProp(cur_node, BAD_CAST "id");
			if (value)
			{
				sscanf(value, "%d\n", &apollo_config.user_id);
				LOG("xml user_id=%s", value);
			}

			value = (char*) xmlGetProp(cur_node, BAD_CAST "account_id");
			if (value)
			{
				sscanf(value, "%lx\n", &apollo_config.account_id);
				LOG("xml account_id=%s", value);
			}
		}

	}
}

xmlNode* _get_owner_node(xmlNode *a_node, xmlChar *owner_name)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if ((xmlStrcasecmp(cur_node->name, BAD_CAST "owner") == 0) && (xmlStrcmp(xmlGetProp(cur_node, BAD_CAST "name"), owner_name) == 0))
            return cur_node;
    }

    return NULL;
}

/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
int read_xml_owner(const char *xmlfile, const char *owner)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *cur_node = NULL;

    /*parse the file and get the DOM */
    doc = xmlParseFile(xmlfile);

    if (doc == NULL) {
        LOG("XML error: could not parse file %s", xmlfile);
        return -1;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);
    cur_node = _get_owner_node(root_element->children, BAD_CAST owner);

    LOG("Setting Owner [%s]...", owner);
    if (cur_node)
        set_xml_owner(cur_node->children);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return 0;
}

/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
char** get_xml_owners(const char *xmlfile)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *cur_node = NULL;
    char *value;
    int count = 1;
    char** ret = NULL;

    /*parse the file and get the DOM */
    doc = xmlParseFile(xmlfile);

    if (doc == NULL)
    {
        LOG("XML: could not parse file %s", xmlfile);

        ret = calloc(1, sizeof(char*) * 2);
        ret[0] = strdup(DEFAULT_USERNAME);

        return ret;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if ((xmlStrcasecmp(cur_node->name, BAD_CAST "owner") == 0) && xmlGetProp(cur_node, BAD_CAST "name"))
            count++;
    }

    ret = calloc(1, sizeof(char*) * count);
    count = 0;

    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcasecmp(cur_node->name, BAD_CAST "owner") == 0)
        {
            value = (char*) xmlGetProp(cur_node, BAD_CAST "name");
            if (value)
            {
                LOG("Adding Owner=%s", value);
                ret[count] = strdup(value);
                count++;
            }
        }
    }

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return ret;
}

int save_xml_owner(const char *xmlfile, const char *username)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */
    char buff[SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE+1];

    /*parse the file and get the DOM */
    doc = xmlReadFile(xmlfile, NULL, XML_PARSE_NONET | XML_PARSE_NOBLANKS);

    snprintf(buff, sizeof(buff), "%s", (username ? username : DEFAULT_USERNAME));

    if (doc)
    {
        /*Get the root element node */
        root_node = xmlDocGetRootElement(doc);
        node = _get_owner_node(root_node->children, BAD_CAST buff);

        if (node)
        {
            xmlUnlinkNode(node);
            xmlFreeNode(node);
        }
    }
    else
    {
        LOG("XML: could not parse data, creating new file '%s'", xmlfile);

        /* 
        * Creates a new document, a node and set it as a root node
        */
        doc = xmlNewDoc(BAD_CAST "1.0");
        root_node = xmlNewNode(NULL, BAD_CAST "apollo");
        xmlNewProp(root_node, BAD_CAST "version", BAD_CAST APOLLO_VERSION);
        xmlDocSetRootElement(doc, root_node);
    }

    /* 
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node. 
     */
    node = xmlNewChild(root_node, NULL, BAD_CAST "owner", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST buff);

	node1 = xmlNewChild(node, NULL, BAD_CAST "console", NULL);

    sprintf(buff, "%016lX %016lX", apollo_config.idps[0], apollo_config.idps[1]);
    xmlNewProp(node1, BAD_CAST "idps", BAD_CAST buff);

    sprintf(buff, "%016lX %016lX", apollo_config.psid[0], apollo_config.psid[1]);
    xmlNewProp(node1, BAD_CAST "psid", BAD_CAST buff);

	node1 = xmlNewChild(node, NULL, BAD_CAST "user", NULL);

    sprintf(buff, "%08d", apollo_config.user_id);
    xmlNewProp(node1, BAD_CAST "id", BAD_CAST buff);

    sprintf(buff, "%016lx", apollo_config.account_id);
    xmlNewProp(node1, BAD_CAST "account_id", BAD_CAST buff);

    /* 
     * Dumping document to file
     */
    xmlSaveFormatFileEnc(xmlfile, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return(0);
}

char* get_xml_title_name(const char *xmlfile)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *cur_node = NULL;
    char *ret = NULL;

    /*parse the file and get the DOM */
    doc = xmlParseFile(xmlfile);
    if (!doc)
    {
        LOG("XML: could not parse file %s", xmlfile);
        return NULL;
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcasecmp(cur_node->name, BAD_CAST "name") == 0)
        {
            ret = strdup((char*) xmlNodeGetContent(cur_node));
            break;
        }
    }

    /*free the document */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return ret;
}
