/************************************************************************/
/* Copyright (c) 2018 CBrain A/S. Version modified from original version by Cristian Adam
 * Copyright (c) 2008 Cristian Adam.

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.

/************************************************************************/

#include "stdafx.h"
#include "ManifestWriter.h"
#include "SHA1.h"

/*

<assembly>                          Yes
    manifestVersion             Yes
<noInheritable>                 No
<assemblyIdentity>                 Yes
    type                         Yes
    name                         Yes
    language                     No
    processorArchitecture         No
    version                     Yes
    publicKeyToken                 No
<dependency>                     No
<dependentAssembly>             No
<file>                             No
    name                         Yes
    hashalg                     No
    hash                         No
<comClass>                         No
    description                 No
    clsid                         Yes
    threadingModel                 No
    tlbid                         No
    progid                         No
    miscStatus                     No
    miscStatusIcon                 No
    miscStatusContent             No
    miscStatusDocPrint             No
    miscStatusDocPrint             No
<typelib>                         No
    tlbid                         Yes
    version                     Yes
    helpdir                     Yes
    resourceid                     No
    flags                         No
<comInterfaceExternalProxyStub>    No
    iid                         Yes
    baseInterface                 No
    numMethods                     No
    name                         No
    tlbid                         No
    proxyStubClsid32             No
<comInterfaceProxyStub>         No
    iid                         Yes
    name                         Yes
    tlbid                         No
    baseInterface                 No
    numMethods                     No
    proxyStubClsid32             No
    threadingModel                 No
<windowClass>                     No
    versioned                     No

*/

ManifestWriter::ManifestWriter(const std::wstring& assemblyName, const std::wstring& assemblyVersion):
CLSID(L"HKEY_CLASSES_ROOT\\CLSID\\"),
INTERFACE(L"HKEY_CLASSES_ROOT\\INTERFACE\\"),
TYPELIB(L"HKEY_CLASSES_ROOT\\TYPELIB\\"),
HKCU_SOFTWARE_CLASSES(L"HKEY_CURRENT_USER\\SOFTWARE\\CLASSES\\"),
GUID_LENGTH(38) // {00000000-0000-0000-0000-000000000000}
{
    m_data << L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;
    m_data << L"<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">" << std::endl;

    m_data << std::endl;
    
    m_data << L"<assemblyIdentity" << std::endl;
    m_data << L"    type=\"win32\"" << std::endl;
    m_data << L"    name=\"" << assemblyName << L"\"" << std::endl;
    m_data << L"    version=\"" << assemblyVersion << L"\" />" << std::endl;

    m_data << std::endl;
}

std::wstring DoubleBackSlash(const std::wstring& fileName)
{
    std::wstring newFileName = fileName;

    std::string::size_type pos = std::wstring::npos;
    std::string::size_type next_pos = 0;
    while ((pos = newFileName.find(L"\\", next_pos)) != std::wstring::npos)
    {
        newFileName.replace(pos, 1, L"\\\\");
        next_pos = pos + 2;
    }

    return newFileName;
}

void ManifestWriter::AddFileSection(const std::wstring& fileName, bool generateHash)
{
    m_data << L"<file name=\"" << DoubleBackSlash(fileName) << L"\"";

    if (!generateHash)
    {
        m_data << L">" << std::endl;
    }
    else
    {
        CSHA1 sha1;

        sha1.Reset();
        sha1.HashFile(fileName.c_str());
        sha1.Final();

        std::wstring hash;
        hash.resize(40);

        sha1.ReportHash(&*hash.begin());

        m_data << L" hash=\"" << hash << L"\" hashalg=\"SHA1\">" << std::endl;
    }

    m_data << std::endl;
        
}

void ManifestWriter::AddComClass( const ComClass& comClass )
{
    m_data << L"    <comClass" << std::endl;
    if (!comClass.description.empty())
    {
        m_data << L"        description=\"" << comClass.description << L"\"" << std::endl;
    }
    m_data << L"        clsid=\"" << comClass.clsid << "\"";

    if (!comClass.threadingModel.empty())
    {
        m_data << std::endl;
        m_data << L"        threadingModel=\"" << comClass.threadingModel << L"\"";
    }

    if (!comClass.progid.empty())
    {
        m_data << std::endl;
        m_data << L"        progid=\"" << comClass.progid << L"\"";
    }

    if (!comClass.tlbid.empty())
    {
        m_data << std::endl;
        m_data << L"        tlbid=\"" << comClass.tlbid << L"\"";
    }
        
    m_data << " />" << std::endl;

    m_data << std::endl;
}

void ManifestWriter::AddTypeLibrary( const TypeLib& typeLib )
{
    m_data << L"    <typelib tlbid=\"" << typeLib.tlbid << L"\"" << std::endl;
    m_data << L"        version=\"" << typeLib.version << L"\"" << std::endl;
    m_data << L"        helpdir=\"" << typeLib.helpdir << L"\" />" << std::endl;

    m_data << std::endl;
}

void ManifestWriter::AddInterface(const Interface& intf)
{
    m_data << L"<comInterfaceExternalProxyStub" << std::endl;
    m_data << L"    name=\"" << intf.name << L"\"" << std::endl;
    m_data << L"    iid=\"" << intf.iid << L"\"" << std::endl;
    m_data << L"    proxyStubClsid32=\"" << intf.proxyStubClsid32 << L"\"" << std::endl;
    m_data << L"    baseInterface=\"" << intf.baseInterface << L"\"" << std::endl;

    if (!intf.tlbid.empty())
    {
        m_data << L"    tlbid=\"" << intf.tlbid << L"\"";
    }

    if (!intf.numMethods.empty())
    {
        if (!intf.tlbid.empty())
        {
            m_data << std::endl;
        }

        m_data << L"    numMethods=\"" << intf.numMethods << L"\"";
    }
    m_data << L" />" << std::endl;

    m_data << std::endl;
}

template<class _T1, class _T2>
struct pair_hash {
    inline std::size_t operator()(const std::pair<_T1, _T2> & v) const {
        size_t h = std::hash<_T1>{}(v.first);
        h ^= std::hash<_T2>{}(v.second) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

void ManifestWriter::ProcessData(const Interceptor::ValuesListType& interceptedValues)
{
    Interceptor::ValuesListType::const_iterator it = interceptedValues.begin();

    std::unordered_map<std::wstring, ComClass> comClasses;
    std::unordered_map<std::pair<std::wstring, std::wstring>, TypeLib, pair_hash<std::wstring, std::wstring>> typeLibs;
    std::unordered_map<std::wstring, Interface> interfaces;

    while (it != interceptedValues.end())
    {
        std::wstring path = it->first;
        std::transform(path.begin(), path.end(), path.begin(), std::towupper);
        if (path.compare(0, HKCU_SOFTWARE_CLASSES.length(), HKCU_SOFTWARE_CLASSES) == 0)
        {
            path = L"HKEY_CLASSES_ROOT\\" + path.substr(HKCU_SOFTWARE_CLASSES.length());
        }
        if (path.compare(0, CLSID.length(), CLSID) == 0)
        {
            std::wstring clsid = path.substr(CLSID.size(), GUID_LENGTH);
            std::wstring subPath = path.length() > CLSID.size() + GUID_LENGTH ? path.substr(CLSID.size() + GUID_LENGTH + 1) : L"";

            if (subPath.compare(0, 10, L"INSTANCE\\{") == 0)
            {
                // Ignore the directshow source filter information
                ++it;
                continue;
            }
            
            ComClass &comClass = comClasses[clsid];
            comClass.clsid = clsid;
            if (subPath.empty() && it->second.first == L"(default)")
            {
                comClass.description = it->second.second;
            }
            else if (subPath.compare(L"PROGID") == 0 && it->second.first == L"(default)")
            {
                comClass.progid = it->second.second;
            }
            else if (subPath.compare(L"TYPELIB") == 0 && it->second.first == L"(default)")
            {
                comClass.tlbid = it->second.second;
            }
            else if (subPath.compare(L"INPROCSERVER32") == 0 && it->second.first == L"ThreadingModel")
            {
                comClass.threadingModel = it->second.second;
            }
        }

        if (path.compare(0, TYPELIB.length(), TYPELIB) == 0)
        {
            std::wstring tlbid = path.substr(TYPELIB.size(), GUID_LENGTH);
            std::wstring subPath = path.length() > TYPELIB.size() + GUID_LENGTH ? path.substr(TYPELIB.size() + GUID_LENGTH + 1) : L"";

            if (subPath.empty()) // no version
                continue;
            std::wstring version = subPath;
            size_t verEndPos = subPath.find(L'\\');
            std::wstring versionSubPath;
            if (verEndPos != std::wstring::npos)
            {
                versionSubPath = version.substr(verEndPos + 1);
                version.resize(verEndPos);
            }

            TypeLib &typeLib = typeLibs[{ tlbid, version }];
            typeLib.tlbid = tlbid;
            typeLib.version = version;
            if (versionSubPath.compare(L"HELPDIR") == 0 && it->second.first == L"(default)")
            {
                typeLib.helpdir = it->second.second;
            }
        }
        if (path.compare(0, INTERFACE.length(), INTERFACE) == 0)
        {
            std::wstring iid = path.substr(INTERFACE.size(), GUID_LENGTH);
            std::wstring subPath = path.length() > INTERFACE.size() + GUID_LENGTH ? path.substr(INTERFACE.size() + GUID_LENGTH + 1) : L"";

            Interface &iface = interfaces[iid];
            iface.iid = iid;
            if (subPath.empty() && it->second.first == L"(default)")
            {
                iface.name = it->second.second;
            }
            if (subPath.compare(L"PROXYSTUBCLSID32") == 0 && it->second.first == L"(default)")
            {
                iface.proxyStubClsid32 = it->second.second;
            }
            else if (subPath.compare(L"TYPELIB") == 0 && it->second.first == L"(default)")
            {
                iface.tlbid = it->second.second;
            }
            else if (subPath.compare(L"NUMMETHODS") == 0 && it->second.first == L"(default)")
            {
                iface.numMethods = it->second.second;
            }
        }

        ++it;
    }

    for (auto ccIt = comClasses.begin(); ccIt != comClasses.end(); ccIt++)
    {
        AddComClass(ccIt->second);
    }

    for (auto tlIt = typeLibs.begin(); tlIt != typeLibs.end(); tlIt++)
    {
        AddTypeLibrary(tlIt->second);
    }

    AddEndFileSection();

    for (auto ifIt = interfaces.begin(); ifIt != interfaces.end(); ifIt++)
    {
        AddInterface(ifIt->second);
    }
}

void ManifestWriter::WriteToFile(const std::wstring& outputManifestFile)
{
    m_data << L"</assembly>" << std::endl;

    FILE* file = _wfopen(outputManifestFile.c_str(), L"w, ccs=utf-8");
    if (file)
    {
        fwrite(&*m_data.str().begin(), 1, m_data.str().size() * 2, file);
        fclose(file);
    }
}

void ManifestWriter::AddEndFileSection()
{
    m_data << L"</file>" << std::endl;

    m_data << std::endl;
}

void ManifestWriter::WriteClientManifest(const std::wstring& clientFileName, const std::vector<DependencyInfo>& dependencyList)
{
    std::wstring executableFileName = clientFileName.substr(0, clientFileName.find(L".manifest"));
    std::ifstream clientFile(executableFileName.c_str(), std::ios_base::binary);

    std::wstring manifestFromExecutable;

    if (!clientFile.fail())
    {
        // Read the manifest from the end of the file
        std::vector<char> clientFileBytes;

        clientFile.seekg(0, std::ios::end);
        clientFileBytes.resize(clientFile.tellg());

        clientFile.seekg(std::ios::beg);
        clientFile.read(&*clientFileBytes.begin(), clientFileBytes.size());
     
        std::vector<char>::iterator rightToTheEnd = clientFileBytes.begin();
        std::advance(rightToTheEnd, max(0, clientFileBytes.size() - 10 * 1024));

        std::string patternBegin = "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">";
        std::vector<char>::iterator posBegin =  std::search(rightToTheEnd, clientFileBytes.end(), patternBegin.begin(), patternBegin.end());

        std::string patternEnd = "</assembly>";
        std::vector<char>::iterator posEnd =  std::search(rightToTheEnd, clientFileBytes.end(), patternEnd.begin(), patternEnd.end());

        if (posBegin != clientFileBytes.end() && posEnd != clientFileBytes.end())
        {
            std::vector<char> manifest;
            std::copy(posBegin, posEnd, std::back_inserter(manifest));

            // Transform it from UTF-8 in wide chars
            int chars = ::MultiByteToWideChar(CP_UTF8, 0, &*manifest.begin(), manifest.size(), 0, 0);
            manifestFromExecutable.resize(chars);

            ::MultiByteToWideChar(CP_UTF8, 0, &*manifest.begin(), manifest.size(), &*manifestFromExecutable.begin(), chars);

            std::string::size_type pos = std::wstring::npos;
            while ((pos = manifestFromExecutable.find(L"\r\n")) != std::wstring::npos)
            {
                manifestFromExecutable.replace(pos, 2, L"\n");
            }
        }
    }


    std::wostringstream client;

    client << L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;

    if (manifestFromExecutable.empty())
    {
        client << L"<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">" << std::endl;

        client << std::endl;

        client << L"<assemblyIdentity" << std::endl;
        client << L"  type=\"win32\"" << std::endl;
        client << L"  name=\"client\"" << std::endl;
        client << L"  version=\"1.0.0.0\" />" << std::endl;
    }
    else
    {
        client << manifestFromExecutable;
    }

    std::vector<DependencyInfo>::const_iterator it;

    it = dependencyList.begin();
    while (it != dependencyList.end())
    {
        client << L"  <dependency>" << std::endl;
        client << L"          <dependentAssembly>" << std::endl;
        client << L"              <assemblyIdentity" << std::endl;
        client << L"                  type=\"win32\"" << std::endl;
        client << L"                  name=\"" << it->assemblyName << L"\"" << std::endl;
        client << L"                  version=\"" << it->assemblyVersion << L"\" />" << std::endl;
        client << L"          </dependentAssembly>" << std::endl;
        client << L"  </dependency>" << std::endl;

        ++it;
    }

    client << L"</assembly>" << std::endl;

    FILE* file = _wfopen(clientFileName.c_str(), L"w, ccs=utf-8");
    if (file)
    {
        fwrite(&*client.str().begin(), 1, client.str().size() * 2, file);
        fclose(file);
    }
}
