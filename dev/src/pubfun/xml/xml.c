/*************************************************************************
*	文 件 名: xml.c
*	功    能: XML链表操作封装函数库
*	编程人员: 
*	编码时间: 2008/9/1
*	修改人员: 
*	修改时间: 
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
  **  函数名称: XmlSetError
  **  功能描述: 保存Xml函数的出错信息
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
  **  返回结果:
  **  备    注: 此函数仅在XML链表操作封装函数库内部使用
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
  **  函数名称: XmlSaveError
  **  功能描述: 保存mxml库函数的出错信息
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                const char * psMessage -- (I), 出错信息
  **  返回结果:
  **  备    注: 此函数仅供mxml函数库调用
  ---------------------------------------------------------------------------*/
void XmlSaveError(const char * psMessage )
{
	memset( l_sXmlError,0x00,sizeof(l_sXmlError) );
	memcpy( l_sXmlError,psMessage,((strlen(psMessage)>=sizeof(l_sXmlError))?(sizeof(l_sXmlError)-1):(strlen(l_sXmlError))) );

	return ;
}

/*----------------------------------------------------------------------------
  **  函数名称: XmlInit
  **  功能描述: 初始化XML链表
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
  **  返回结果:
  				 ==0   -- 成功
  				 !=0   -- 失败
  **  备    注:
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
  **  函数名称: XmlFree
  **  功能描述: 释放XML链表所占用的内存
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
  **  返回结果:
  				 ==0   -- 成功
  				 !=0   -- 失败
  **  备    注:
  ---------------------------------------------------------------------------*/
void XmlFree( )
{
	mxmlDelete( l_pIntXml );
	l_pIntXml = NULL;

	return;
}

/*----------------------------------------------------------------------------
  **  函数名称: XmlGet
  **  功能描述: 从XML链表中取出一个元素标签的值
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                char * psData    -- (O), 标签值缓冲区
                int nBuffSize    -- (I), 缓冲区大小
  **  返回结果:
  				 >= 0 -- 成功, 实际标签值长度
  				 < 0  -- 失败
  **  备    注: 在向psData写入数据前, 先往psData写入nBuffSize个0x00,往psData中
                写入的数据不超过nBuffSize-1个bytes
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
  **  函数名称: XmlIntGet
  **  功能描述: 从XML链表中取出一个整数值
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                int  * pnData    -- (O), 标签值缓冲区
  **  返回结果:
  				 >= 0 -- 成功, 实际标签值长度
  				 < 0  -- 失败
  **  备    注: 在向psData写入数据前, 先往psData写入nBuffSize个0x00,往psData中
                写入的数据不超过nBuffSize-1个bytes
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
  **  函数名称: XmlSet
  **  功能描述: 在XML链表中写入一个元素标签的值
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                char * psData    -- (I), 标签值
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlCharSet
  **  功能描述: 在XML链表中写入单个字符
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                char   cData     -- (I), 标签字符值
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlIntSet
  **  功能描述: 在XML链表中写入整数
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                int    nData     -- (I), 整数值
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlAddList
  **  功能描述: 在XML链表中写入一个明细元素
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psRecordSetName -- (I), 记录集标签名
                int    nRecID          -- (I), 记录序号, 从1开始
                char * psEmName        -- (I), 明细标签名
                char * psData          -- (I), 明细标签值
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlAddIntList
  **  功能描述: 在XML链表中写入一个整数值的明细元素
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psRecordSetName -- (I), 记录集标签名
                int    nRecID          -- (I), 记录序号, 从1开始
                char * psEmName        -- (I), 明细标签名
                char * psData          -- (I), 明细标签值
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlFetchList
  **  功能描述: 从XML链表中一个明细元素的属性值
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psRecordSetName -- (I), 记录集标签名
                int    nRecID          -- (I), 记录序号, 从1开始
                char * psEmName        -- (I), 明细标签名
                char * psData          -- (I), 明细标签值
                int nBuffSize          -- (I), 缓冲区大小
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlLoadString
  **  功能描述: 将符合内部XML规范的XML字符串解析到Xml链表中
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psData -- (I), 符合内部XML规范的XML字符串
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
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
  **  函数名称: XmlSaveString
  **  功能描述: 将内部XML链表输出到字符串中
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psData -- (O), XML字符串
                int    nSize  -- (I), XML字符串的大小
  **  返回结果:
  				 >  0  -- 成功,XML字符串的实际长度
  				 <= 0  -- 失败
  **  备    注: 
  ---------------------------------------------------------------------------*/
int XmlSaveString( char * psData, int nSize )
{
	return mxmlSaveString( l_pIntXml,psData, nSize, MXML_NO_CALLBACK );
}

/*----------------------------------------------------------------------------
  **  函数名称: XmlStringCmp
  **  功能描述: 元素值与传入的参数作字符串比较
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName -- (I), 标签名
                char * psData    -- (I), 标签值缓冲区
  **  返回结果:
  				 = 0 -- 元素值与传入的参数相同
  				 < 0 -- 元素值小于传入的参数或失败
  				 > 0 -- 元素值小于传入的参数
  **  备    注: 
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
  **  函数名称: XmlElementCmp
  **  功能描述: 比较两个元素是否相同
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                char * psTagName1 -- (I), 标签名1
                char * psTagName2 -- (I), 标签名2
  **  返回结果:
  				 = 0 -- 元素1等于元素2
  				 < 0 -- 元素1小于元素2或失败
  				 > 0 -- 元素1大于元素2
  **  备    注: 
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
  **  函数名称: XmlShow
  **  功能描述: 显示XML链表内容
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
                FILE * fp -- (I), 写入文件
  **  返回结果:
  				 == 0 -- 成功
  				 < 0  -- 失败
  **  备    注: 
  ---------------------------------------------------------------------------*/
int XmlShow( FILE * fp )
{
	return mxmlSaveFile( l_pIntXml, fp, 0 );
}

/*----------------------------------------------------------------------------
  **  函数名称: XmlError
  **  功能描述: 取出Xml函数的出错信息
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
  **  返回结果:
  				 Xml函数的出错信息
  **  备    注:
  ---------------------------------------------------------------------------*/
char * XmlError()
{
	return l_sXmlError;
}

/*----------------------------------------------------------------------------
  **  函数名称: XmlPointer
  **  功能描述: 取出内部Xml链表的头指针
  **  作    者: 
  **  编写日期:
  **  修    改:	
  **  修改日期:
  **  参    数: 
  **  返回结果:
  				 Xml链表的头指针
  **  备    注:
  ---------------------------------------------------------------------------*/
void * XmlPointer()
{
	return (void*)l_pIntXml;
}
