/** 
 * XML Security Library example: Encrypting data using a template file.
 * 
 * Encrypts binary data using a template file and a DES key from a binary file
 * 
 * Usage: 
 *	enc1 <xml-tmpl> <des-key-file> 
 *
 * Example:
 *	./enc1 ./enc1-tmpl.xml deskey.bin > enc1-res.xml
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 * 
 * Copyrigth (C) 2002-2003 Aleksey Sanin <aleksey@aleksey.com>
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifndef XMLSEC_NO_XSLT
#include <libxslt/xslt.h>
#endif /* XMLSEC_NO_XSLT */

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/keys.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/xmlenc.h>
#include <xmlsec/crypto.h>

int encrypt_file(const char* tmpl_file, const char* key_file, 
		 const unsigned char* data, size_t dataSize);
int 
main(int argc, char **argv) {
    static const char secret_data[] = "Big secret";
    
    assert(argv);

    if(argc != 3) {
	fprintf(stderr, "Error: wrong number of arguments.\n");
	fprintf(stderr, "Usage: %s <tmpl-file> <key-file>\n", argv[0]);
	return(1);
    }

    /* Init libxml and libxslt libraries */
    xmlInitParser();
    LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);
#ifndef XMLSEC_NO_XSLT
    xmlIndentTreeOutput = 1; 
#endif /* XMLSEC_NO_XSLT */
        	
    /* Init xmlsec library */
    if(xmlSecInit() < 0) {
	fprintf(stderr, "Error: xmlsec initialization failed.\n");
	return(-1);
    }

    /* Init crypto library */
    if(xmlSecCryptoAppInit(NULL) < 0) {
	fprintf(stderr, "Error: crypto initialization failed.\n");
	return(-1);
    }

    /* Init xmlsec-crypto library */
    if(xmlSecCryptoInit() < 0) {
	fprintf(stderr, "Error: xmlsec-crypto initialization failed.\n");
	return(-1);
    }

    if(encrypt_file(argv[1], argv[2], secret_data, strlen(secret_data)) < 0) {
	return(-1);
    }    
    
    /* Shutdown xmlsec-crypto library */
    xmlSecCryptoShutdown();
    
    /* Shutdown crypto library */
    xmlSecCryptoAppShutdown();
    
    /* Shutdown xmlsec library */
    xmlSecShutdown();

    /* Shutdown libxslt/libxml */
#ifndef XMLSEC_NO_XSLT
    xsltCleanupGlobals();            
#endif /* XMLSEC_NO_XSLT */
    xmlCleanupParser();
    
    return(0);
}

int 
encrypt_file(const char* tmpl_file, const char* key_file, const unsigned char* data, size_t dataSize) {
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;
    xmlSecEncCtxPtr encCtx = NULL;
    int res = -1;
    
    assert(tmpl_file);
    assert(key_file);
    assert(data);

    /* load template */
    doc = xmlParseFile(tmpl_file);
    if ((doc == NULL) || (xmlDocGetRootElement(doc) == NULL)){
	fprintf(stderr, "Error: unable to parse file \"%s\"\n", tmpl_file);
	goto done;	
    }
    
    /* find start node */
    node = xmlSecFindNode(xmlDocGetRootElement(doc), xmlSecNodeEncryptedData, xmlSecEncNs);
    if(node == NULL) {
	fprintf(stderr, "Error: start node not found in \"%s\"\n", tmpl_file);
	goto done;	
    }

    /* create encryption context, we don't need keys manager in this example */
    encCtx = xmlSecEncCtxCreate(NULL);
    if(encCtx == NULL) {
        fprintf(stderr,"Error: failed to create encryption context\n");
	goto done;
    }

    /* load DES key, assuming that there is not password */
    encCtx->encKey = xmlSecKeyReadBinaryFile(xmlSecKeyDataDesId, key_file);
    if(encCtx->encKey == NULL) {
        fprintf(stderr,"Error: failed to load des key from binary file \"%s\"\n", key_file);
	goto done;
    }

    /* encrypt the data */
    if(xmlSecEncCtxBinaryEncrypt(encCtx, node, data, dataSize) < 0) {
        fprintf(stderr,"Error: encryption failed\n");
	goto done;
    }
        
    /* print encrypted data with document to stdout */
    xmlDocDump(stdout, doc);
    
    /* success */
    res = 0;

done:    

    /* cleanup */
    if(encCtx != NULL) {
	xmlSecEncCtxDestroy(encCtx);
    }
    
    if(doc != NULL) {
	xmlFreeDoc(doc); 
    }
    return(res);
}

