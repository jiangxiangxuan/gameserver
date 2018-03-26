
#include "KernalConfig.h"
#include "KernalUtil.h"

KernalConfig::KernalConfig()
	:m_pDocument( NULL ),
	 m_pHandle( NULL )
{

}

KernalConfig::~KernalConfig()
{
	if( m_pHandle )
	{
		delete m_pHandle;
		m_pHandle = NULL;
	}
	if( m_pDocument )
	{
		delete m_pDocument;
		m_pDocument = NULL;
	}
}

bool KernalConfig::init(const char *path)
{
    m_pDocument = new TiXmlDocument( path );
    m_pDocument->LoadFile();
    m_pHandle = new TiXmlHandle( m_pDocument );
	return true;
}

int KernalConfig::getAttributeInt(const char *nodePath, const char *attr)
{
	int result = 0;
	std::vector<std::string> nodes = strSplit(nodePath, "/");

	TiXmlHandle   handle = *m_pHandle;
	TiXmlElement *pElement = NULL;
	for( auto iter = nodes.begin(); iter != nodes.end(); iter++ )
	{
		handle = handle.FirstChild((*iter).c_str());
	}
	pElement = handle.ToElement();

	pElement->QueryIntAttribute(attr, &result);
	return result;
}

const char *KernalConfig::getAttributeStr(const char *nodePath, const char *attr)
{
	std::vector<std::string> nodes = strSplit(nodePath, "/");

	TiXmlHandle   handle = *m_pHandle;
	TiXmlElement *pElement = NULL;
	for( auto iter = nodes.begin(); iter != nodes.end(); iter++ )
	{
		handle = handle.FirstChild((*iter).c_str());
	}
	pElement = handle.ToElement();

	return pElement->Attribute(attr);
}

const char *KernalConfig::getText(const char *nodePath)
{
	std::vector<std::string> nodes = strSplit(nodePath, "/");

	TiXmlHandle   handle = *m_pHandle;
	TiXmlElement *pElement = NULL;
	for( auto iter = nodes.begin(); iter != nodes.end(); iter++ )
	{
		handle = handle.FirstChild((*iter).c_str());
	}
	pElement = handle.ToElement();

	return pElement->GetText();
}
