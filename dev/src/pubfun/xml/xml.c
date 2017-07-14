/*************************************************************************
*	�� �� ��: xml.c
*	��    ��: XML���������װ������
*	�����Ա: 
*	����ʱ��: 2008/9/1
*	�޸���Ա: 
*	�޸�ʱ��: 
* 	Copyright (c) 1999 by Cintech Corp. All rights reserved.
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "mxml.h"
#include "xml.h"

static mxml_node_t * l_pIntXml=NULL;
static char l_sXmlError[1024];

/*----------------------------------------------------------------------------
  **  ��������: XmlSetError
  **  ��������: ����Xml�����ĳ�����Ϣ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
  **  ���ؽ��:
  **  ��    ע: �˺�������XML���������װ�������ڲ�ʹ��
  ---------------------------------------------------------------------------*/
void XmlSetError(char * pcFmt, ... )
{
	va_list args;

	strcpy( l_sXmlError, "error: " );

	va_start( args, pcFmt );
	vsprintf( l_sXmlError+7, pcFmt, args );
	va_end( args );

	return ;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlSaveError
  **  ��������: ����mxml�⺯���ĳ�����Ϣ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                const char * psMessage -- (I), ������Ϣ
  **  ���ؽ��:
  **  ��    ע: �˺�������mxml���������
  ---------------------------------------------------------------------------*/
void XmlSaveError(const char * psMessage )
{
	memset( l_sXmlError,0x00,sizeof(l_sXmlError) );
	memcpy( l_sXmlError,psMessage,((strlen(psMessage)>=sizeof(l_sXmlError))?(sizeof(l_sXmlError)-1):(strlen(l_sXmlError))) );

	return ;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlInit
  **  ��������: ��ʼ��XML����
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
  **  ���ؽ��:
  				 ==0   -- �ɹ�
  				 !=0   -- ʧ��
  **  ��    ע:
  ---------------------------------------------------------------------------*/
int XmlInit(  )
{
	mxml_node_t * node;
	mxmlSetErrorCallback( XmlSaveError );

	if( l_pIntXml )
		mxmlDelete( l_pIntXml );

	l_pIntXml = mxmlNewElement(NULL, "?xml version=\"1.0\" \"encoding\"=\"GB18030\"?");
	if( l_pIntXml == NULL )
	{
		XmlSetError( "Unable to initialize XML tree!" );
		return (-1);
	}

	if( (node = mxmlNewElement(l_pIntXml,"Message")) == NULL )
	{
		mxmlDelete( l_pIntXml );
		XmlSetError( "Unable to add <Message> element in XML tree!" );
		return (-1);
	}

	if( mxmlNewElement(node,"Public") == NULL )
	{
		mxmlDelete( l_pIntXml );
		XmlSetError( "Unable to add <Public> element in XML tree!" );
		return (-1);
	}

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlFree
  **  ��������: �ͷ�XML������ռ�õ��ڴ�
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
  **  ���ؽ��:
  				 ==0   -- �ɹ�
  				 !=0   -- ʧ��
  **  ��    ע:
  ---------------------------------------------------------------------------*/
void XmlFree( )
{
	mxmlDelete( l_pIntXml );
	l_pIntXml = NULL;

	return;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlGet
  **  ��������: ��XML������ȡ��һ��Ԫ�ر�ǩ��ֵ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                char * psData    -- (O), ��ǩֵ������
                int nBuffSize    -- (I), ��������С
  **  ���ؽ��:
  				 >= 0 -- �ɹ�, ʵ�ʱ�ǩֵ����
  				 < 0  -- ʧ��
  **  ��    ע: ����psDataд������ǰ, ����psDataд��nBuffSize��0x00,��psData��
                д������ݲ�����nBuffSize-1��bytes
  ---------------------------------------------------------------------------*/
int XmlGet(char * psTagName,char * psData,int nBuffSize )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char * psAttrValue;
	int  nLen;

	memset( psData,0x00,nBuffSize );

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!",psTagName );
		return (-1);
	}

	psAttrValue = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psTagName );
		return (-1);
	}

	memcpy( psData,psAttrValue,((strlen(psAttrValue)>=nBuffSize)?(nBuffSize-1):(strlen(psAttrValue))) );

	return strlen(psAttrValue);
}

/*----------------------------------------------------------------------------
  **  ��������: XmlIntGet
  **  ��������: ��XML������ȡ��һ������ֵ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                int  * pnData    -- (O), ��ǩֵ������
  **  ���ؽ��:
  				 >= 0 -- �ɹ�, ʵ�ʱ�ǩֵ����
  				 < 0  -- ʧ��
  **  ��    ע: ����psDataд������ǰ, ����psDataд��nBuffSize��0x00,��psData��
                д������ݲ�����nBuffSize-1��bytes
  ---------------------------------------------------------------------------*/
int XmlIntGet(char * psTagName, int * pnData )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char * psAttrValue;
	int  nLen;

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!",psTagName );
		return (-1);
	}

	psAttrValue = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psTagName );
		return (-1);
	}

	*pnData = atoi( psAttrValue );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlSet
  **  ��������: ��XML������д��һ��Ԫ�ر�ǩ��ֵ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                char * psData    -- (I), ��ǩֵ
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlSet(char * psTagName,char * psData )
{
	mxml_node_t		* tree,*node;		/* XML tree */

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		node = mxmlNewElement( tree,psTagName );
		if( node == NULL )
		{
			XmlSetError( "Unable to add <%s> element in XML tree!",psTagName );
			return (-1);
		}
	}

	mxmlElementSetAttr( node, "value", psData );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlCharSet
  **  ��������: ��XML������д�뵥���ַ�
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                char   cData     -- (I), ��ǩ�ַ�ֵ
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlCharSet(char * psTagName,char cData )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char sData[2];

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		node = mxmlNewElement( tree,psTagName );
		if( node == NULL )
		{
			XmlSetError( "Unable to add <%s> element in XML tree!",psTagName );
			return (-1);
		}
	}

	memset( sData, 0x00, sizeof(sData) );
	*sData = cData;
	mxmlElementSetAttr( node, "value", sData );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlIntSet
  **  ��������: ��XML������д������
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                int    nData     -- (I), ����ֵ
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlIntSet(char * psTagName, int nData )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char sData[11];

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		node = mxmlNewElement( tree,psTagName );
		if( node == NULL )
		{
			XmlSetError( "Unable to add <%s> element in XML tree!",psTagName );
			return (-1);
		}
	}

	memset( sData, 0x00, sizeof(sData) );
	sprintf( sData, "%d", nData );
	mxmlElementSetAttr( node, "value", sData );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlAddList
  **  ��������: ��XML������д��һ����ϸԪ��
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psRecordSetName -- (I), ��¼����ǩ��
                int    nRecID          -- (I), ��¼���, ��1��ʼ
                char * psEmName        -- (I), ��ϸ��ǩ��
                char * psData          -- (I), ��ϸ��ǩֵ
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlAddList( char * psRecordSetName,int nRecID,char * psEmName,char * psData)
{
	mxml_node_t		* tree,*node,*recordset,*record;		/* XML tree */
	char sRecID[11];

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <RecordSet> element*/
	recordset = mxmlFindElement(tree, tree, "RecordSet", "name",psRecordSetName, MXML_DESCEND_FIRST);
	if( recordset == NULL )
	{
		recordset = mxmlNewElement( tree,"RecordSet" );
		if( recordset == NULL )
		{
			XmlSetError( "Unable to add <RecordSet> element in XML tree!" );
			return (-1);
		}
		mxmlElementSetAttr( recordset, "name", psRecordSetName );
	}

	/* find <Record> element */
	sprintf( sRecID,"%d",nRecID );
	record = mxmlFindElement(recordset, recordset, "Record", "RecID", sRecID, MXML_DESCEND_FIRST);
	if( record == NULL )
	{
		record = mxmlNewElement( recordset,"Record" );
		if( record == NULL )
		{
			XmlSetError( "Unable to add <Record> element in XML tree!" );
			return (-1);
		}
		mxmlElementSetAttr( record, "RecID", sRecID );
	}

	/* find element */
	node = mxmlFindElement(record, record, psEmName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		node = mxmlNewElement( record,psEmName );
		if( node == NULL )
		{
			XmlSetError( "Unable to add <%s> element in XML tree!", psEmName );
			return (-1);
		}
	}

	mxmlElementSetAttr( node, "value", psData );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlAddIntList
  **  ��������: ��XML������д��һ������ֵ����ϸԪ��
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psRecordSetName -- (I), ��¼����ǩ��
                int    nRecID          -- (I), ��¼���, ��1��ʼ
                char * psEmName        -- (I), ��ϸ��ǩ��
                char * psData          -- (I), ��ϸ��ǩֵ
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlAddIntList( char * psRecordSetName,int nRecID,char * psEmName,int nData)
{
	mxml_node_t		* tree,*node,*recordset,*record;		/* XML tree */
	char sRecID[11];
	char sData[11];

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <RecordSet> element*/
	recordset = mxmlFindElement(tree, tree, "RecordSet", "name",psRecordSetName, MXML_DESCEND_FIRST);
	if( recordset == NULL )
	{
		recordset = mxmlNewElement( tree,"RecordSet" );
		if( recordset == NULL )
		{
			XmlSetError( "Unable to add <RecordSet> element in XML tree!" );
			return (-1);
		}
		mxmlElementSetAttr( recordset, "name", psRecordSetName );
	}

	/* find <Record> element */
	sprintf( sRecID,"%d",nRecID );
	record = mxmlFindElement(recordset, recordset, "Record", "RecID", sRecID, MXML_DESCEND_FIRST);
	if( record == NULL )
	{
		record = mxmlNewElement( recordset,"Record" );
		if( record == NULL )
		{
			XmlSetError( "Unable to add <Record> element in XML tree!" );
			return (-1);
		}
		mxmlElementSetAttr( record, "RecID", sRecID );
	}

	/* find element */
	node = mxmlFindElement(record, record, psEmName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		node = mxmlNewElement( record,psEmName );
		if( node == NULL )
		{
			XmlSetError( "Unable to add <%s> element in XML tree!", psEmName );
			return (-1);
		}
	}

	memset( sData, 0x00, sizeof(sData) );
	sprintf( sData, "%d", nData );
	mxmlElementSetAttr( node, "value", sData );

	return 0;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlFetchList
  **  ��������: ��XML������һ����ϸԪ�ص�����ֵ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psRecordSetName -- (I), ��¼����ǩ��
                int    nRecID          -- (I), ��¼���, ��1��ʼ
                char * psEmName        -- (I), ��ϸ��ǩ��
                char * psData          -- (I), ��ϸ��ǩֵ
                int nBuffSize          -- (I), ��������С
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlFetchList( char * psRecordSetName,int nRecID,char * psEmName,char * psData,int nBuffSize)
{
	mxml_node_t		* tree,*node,*recordset,*record;		/* XML tree */
	char sRecID[11];
	char * psAttrValue;

	memset( psData,0x00,nBuffSize );

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <RecordSet> element*/
	recordset = mxmlFindElement(tree, tree, "RecordSet", "name",psRecordSetName, MXML_DESCEND_FIRST);
	if( recordset == NULL )
	{
		XmlSetError( "Unable to find <RecordSet name=\"%s\"> element in XML tree!", psRecordSetName );
		return -1;
	}

	/* find <Record> element */
	sprintf( sRecID,"%d",nRecID );
	record = mxmlFindElement(recordset, recordset, "Record", "RecID", sRecID, MXML_DESCEND_FIRST);
	if( record == NULL )
	{
		XmlSetError( "Unable to find <Record RecID=\"%d\"> element in XML tree!", nRecID );
		return -1;
	}

	/* find element */
	node = mxmlFindElement(record, record, psEmName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!", psEmName );
		return (-1);
	}

	psAttrValue = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psEmName );
		return (-1);
	}

	memcpy( psData,psAttrValue,((strlen(psAttrValue)>=nBuffSize)?(nBuffSize-1):(strlen(psAttrValue))) );

	return strlen(psAttrValue);
}

/*----------------------------------------------------------------------------
  **  ��������: XmlLoadString
  **  ��������: �������ڲ�XML�淶��XML�ַ���������Xml������
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psData -- (I), �����ڲ�XML�淶��XML�ַ���
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlLoadString( char * psData )
{
	mxmlSetErrorCallback( XmlSaveError );

	l_pIntXml = mxmlLoadString( NULL, psData, MXML_NO_CALLBACK );
	if( l_pIntXml )
		return 0;
	else
		return -1;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlSaveString
  **  ��������: ���ڲ�XML����������ַ�����
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psData -- (O), XML�ַ���
                int    nSize  -- (I), XML�ַ����Ĵ�С
  **  ���ؽ��:
  				 >  0  -- �ɹ�,XML�ַ�����ʵ�ʳ���
  				 <= 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlSaveString( char * psData, int nSize )
{
	return mxmlSaveString( l_pIntXml,psData, nSize, MXML_NO_CALLBACK );
}

/*----------------------------------------------------------------------------
  **  ��������: XmlStringCmp
  **  ��������: Ԫ��ֵ�봫��Ĳ������ַ����Ƚ�
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName -- (I), ��ǩ��
                char * psData    -- (I), ��ǩֵ������
  **  ���ؽ��:
  				 = 0 -- Ԫ��ֵ�봫��Ĳ�����ͬ
  				 < 0 -- Ԫ��ֵС�ڴ���Ĳ�����ʧ��
  				 > 0 -- Ԫ��ֵС�ڴ���Ĳ���
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlStringCmp(char * psTagName,char * psData )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char * psAttrValue;

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element*/
	node = mxmlFindElement(tree, tree, psTagName, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!",psTagName );
		return (-1);
	}

	psAttrValue = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psTagName );
		return (-1);
	}

	return strcmp( psAttrValue, psData );
}

/*----------------------------------------------------------------------------
  **  ��������: XmlElementCmp
  **  ��������: �Ƚ�����Ԫ���Ƿ���ͬ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                char * psTagName1 -- (I), ��ǩ��1
                char * psTagName2 -- (I), ��ǩ��2
  **  ���ؽ��:
  				 = 0 -- Ԫ��1����Ԫ��2
  				 < 0 -- Ԫ��1С��Ԫ��2��ʧ��
  				 > 0 -- Ԫ��1����Ԫ��2
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlElementCmp(char * psTagName1,char * psTagName2 )
{
	mxml_node_t		* tree,*node;		/* XML tree */
	char * psAttrValue1, * psAttrValue2;

	tree = l_pIntXml;

	/* find <Message> element*/
	tree = mxmlFindElement(tree, tree, "Message", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Message> element in XML tree!" );
		return (-1);
	}

	/* find <Public> element*/
	tree = mxmlFindElement(tree, tree, "Public", NULL, NULL, MXML_DESCEND_FIRST);
	if( tree == NULL )
	{
		XmlSetError( "Unable to find <Public> element in XML tree!" );
		return (-1);
	}

	/* find element 1 */
	node = mxmlFindElement(tree, tree, psTagName1, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!",psTagName1 );
		return (-1);
	}

	psAttrValue1 = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue1 == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psTagName1 );
		return (-1);
	}

	/* find element 2 */
	node = mxmlFindElement(tree, tree, psTagName2, NULL, NULL, MXML_DESCEND_FIRST);
	if( node == NULL )
	{
		XmlSetError( "Unable to find <%s> element in XML tree!",psTagName2 );
		return (-1);
	}

	psAttrValue2 = (char *)mxmlElementGetAttr( node,"value" );

	if( psAttrValue2 == NULL )
	{
		XmlSetError( "<%s> element 'value' attribute not exists!", psTagName2 );
		return (-1);
	}

	return strcmp( psAttrValue1, psAttrValue2 );
}

/*----------------------------------------------------------------------------
  **  ��������: XmlShow
  **  ��������: ��ʾXML��������
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
                FILE * fp -- (I), д���ļ�
  **  ���ؽ��:
  				 == 0 -- �ɹ�
  				 < 0  -- ʧ��
  **  ��    ע: 
  ---------------------------------------------------------------------------*/
int XmlShow( FILE * fp )
{
	return mxmlSaveFile( l_pIntXml, fp, 0 );
}

/*----------------------------------------------------------------------------
  **  ��������: XmlError
  **  ��������: ȡ��Xml�����ĳ�����Ϣ
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
  **  ���ؽ��:
  				 Xml�����ĳ�����Ϣ
  **  ��    ע:
  ---------------------------------------------------------------------------*/
char * XmlError()
{
	return l_sXmlError;
}

/*----------------------------------------------------------------------------
  **  ��������: XmlPointer
  **  ��������: ȡ���ڲ�Xml�����ͷָ��
  **  ��    ��: 
  **  ��д����:
  **  ��    ��:	
  **  �޸�����:
  **  ��    ��: 
  **  ���ؽ��:
  				 Xml�����ͷָ��
  **  ��    ע:
  ---------------------------------------------------------------------------*/
void * XmlPointer()
{
	return (void*)l_pIntXml;
}
