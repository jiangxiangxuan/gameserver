
#ifndef _KERNAL_CONFIG_H_
#define _KERNAL_CONFIG_H_

#include "tinyxml/tinyxml.h"

class KernalConfig
{
public:
	KernalConfig();
	~KernalConfig();

	bool init(const char *path);

	int getAttributeInt(const char *nodePath, const char *attr);
	const char *getAttributeStr(const char *nodePath, const char *attr);
	const char *getText(const char *nodePath);
private:
	TiXmlDocument *m_pDocument;
	TiXmlHandle   *m_pHandle;
};

#endif
