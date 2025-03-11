#include "projectconfig.h"
#define CONFIG_BUFFER_SIZE (10)
using namespace tinyxml2;
ProjectConfig::ProjectConfig(const char* project)
{
    std::string sn(project);
    project_ = sn;
    if(m_doc.LoadFile(project) != tinyxml2::XML_SUCCESS){
        createXML(project);
    }
    else{
        m_id = getElementTxt("Root");
        m_serial_number=getElementTxt("serial_number");
    }
}

int ProjectConfig::getElementVal(const char *name)
{
    tinyxml2::XMLElement* root = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(root, name);
    int result = -1;
    if(!element){
        const char* ptr = element->Value();
        tinyxml2::XMLUtil::ToInt(ptr, &result);
    }
    return result;
}

const char *ProjectConfig::getElementTxt(const char *name)
{
    tinyxml2::XMLElement* root = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(root, name);
    if(element)
        return element->GetText();
    else
        return  nullptr;
}

const char *ProjectConfig::getElementTxt(const char* root, const char *name)
{
    tinyxml2::XMLElement* Rt = m_doc.RootElement();
    tinyxml2::XMLElement* father = search(Rt, root);
    tinyxml2::XMLElement* element = search(father, name);
    if(element)
        return element->GetText();
    else
        return  nullptr;
}

void ProjectConfig::setElementTxt(const char* root, const char *name, const char *txt)
{
    tinyxml2::XMLElement* Rt = m_doc.RootElement();
    tinyxml2::XMLElement* father = search(Rt, root);
    tinyxml2::XMLElement* element = search(father, name);
    if(element){
        element->SetText(txt);  
    }
    else{
        std::cout << "error set " << std::endl;
    }
}

void ProjectConfig::insertElementEndChild(const char *root, const char *name, const char *txt)
{
    tinyxml2::XMLElement* r = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(r, root);
    tinyxml2::XMLElement* child = m_doc.NewElement(name);
    child->SetText(txt);
    element->InsertEndChild(child);
}

void ProjectConfig::insertElementEndChild(const char *root, const char *name, int val)
{
    tinyxml2::XMLElement* r = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(r, root);
    tinyxml2::XMLElement* child = m_doc.NewElement(name);
    char buffer[10] = {0};
    tinyxml2::XMLUtil::ToStr(val,  buffer, CONFIG_BUFFER_SIZE);
    child->SetValue(buffer);
    element->InsertEndChild(child);
}

void ProjectConfig::insertElementSlibing(const char *root, const char *name, const char *txt)
{
    tinyxml2::XMLElement* r = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(r, root);
    if(!element){
        tinyxml2::XMLElement* child = m_doc.NewElement(name);
        child->SetText(txt);
        element->InsertAfterChild(element, child);
    }
}

void ProjectConfig::insertElementSlibing(const char *root, const char *name, int val)
{
    tinyxml2::XMLElement* r = m_doc.RootElement();
    tinyxml2::XMLElement* element = search(r, root);
    if(!element){
        tinyxml2::XMLElement* child = m_doc.NewElement(name);
        char buffer[10] = {0};
        tinyxml2::XMLUtil::ToStr(val,  buffer, CONFIG_BUFFER_SIZE);
        child->SetValue(buffer);
        element->InsertEndChild(child);
    }
}

tinyxml2::XMLElement *ProjectConfig::search(tinyxml2::XMLElement* element, const char *name)
{
    tinyxml2::XMLElement* next = element;
    if(element == nullptr){
        return nullptr;
    }
    while(next){
        const char* n = next->Name();
        if(strcmp(n, name) == 0){
            return next;
        }
        else {
            next = next->NextSiblingElement();
        }
    }
    if(next == nullptr){
        tinyxml2::XMLElement* root = element->FirstChildElement();
        if(root){
            return search(root, name);
        }
    }
    return next;
}

int ProjectConfig::createXML(const char *project)
{
    // 添加声明，表示这是一个XML文档
    tinyxml2::XMLDeclaration* declaration = m_doc.NewDeclaration();
    m_doc.InsertEndChild(declaration);

    // 创建根元素
    tinyxml2::XMLElement* root = m_doc.NewElement("Root");
    root->SetText("1773587526036901890");
    m_doc.InsertEndChild(root);

    // 在根元素下添加子元素
    tinyxml2::XMLElement* childElement = m_doc.NewElement("radar_device_info");
    root->InsertEndChild(childElement);
    childElement->SetText(project);

    // 在根元素下添加子元素
    tinyxml2::XMLElement* childElement1 = m_doc.NewElement("radar_alarm_info");
    root->InsertEndChild(childElement1);
    childElement1->SetText(project);
    insertElementEndChild("radar_device_info", RDI_ID, "1773587526036901890");//lz
    insertElementEndChild("radar_device_info", RDI_CREATE_BY, "admin");
    insertElementEndChild("radar_device_info", RDI_CREATE_TIME, "2023-11-01 20:53:44");
    insertElementEndChild("radar_device_info", RDI_UPDATE_BY, "test");
    insertElementEndChild("radar_device_info", RDI_UPDATE_TIME, "2023-11-08 00:48:31");
    insertElementEndChild("radar_device_info", RDI_ORG_CODE, "A02");//lz
    insertElementEndChild("radar_device_info", RDI_NAME, "设备二");//lz
    insertElementEndChild("radar_device_info", RDI_SERIAL_NUMBER, "000002");//lz
    insertElementEndChild("radar_device_info", RDI_SOFT_VERSION_NUMBER, "1.0.2");
    insertElementEndChild("radar_device_info", RDI_HARD_VERSION_NUMBER, "1.0.1");
    insertElementEndChild("radar_device_info", RDI_FACTORY_DATE, "2023-11-01 00:00:00");
    insertElementEndChild("radar_device_info", RDI_HAND_CODE, "0000000002");//lz
    insertElementEndChild("radar_device_info", RDI_IP_ADDRESS, "10.42.0.1");//lz
    insertElementEndChild("radar_device_info", RDI_RATE, "1");//lz
    insertElementEndChild("radar_alarm_info", RAI_ID, "1720659778782662658");//lz
    insertElementEndChild("radar_alarm_info", RAI_CREATE_BY, "admin");
    insertElementEndChild("radar_alarm_info", RAI_CREATE_TIME, "2023-11-01 20:53:44");
    insertElementEndChild("radar_alarm_info", RAI_UPDATE_BY, "test");
    insertElementEndChild("radar_alarm_info", RAI_UPDATE_TIME, "2023-11-08 00:48:31");
    insertElementEndChild("radar_alarm_info", RAI_ORG_CODE, "A02");//lz
    insertElementEndChild("radar_alarm_info", RAI_DEVICE_NAME, "1773587526036901890");//lz
    insertElementEndChild("radar_alarm_info", RAI_DETECTION_RANGE, "3");
    insertElementEndChild("radar_alarm_info", RAI_CLUSTERING_THRESHOLD, "0.08");//lz
    insertElementEndChild("radar_alarm_info", RAI_MAX_COUNT, "5000");//lz 3500
    insertElementEndChild("radar_alarm_info", RAI_MIN_COUNT, "30");//lz
    insertElementEndChild("radar_alarm_info", RAI_TOTAL_TIME, "0.8");//lz
    insertElementEndChild("radar_alarm_info", RAI_TWO_DETECTION_RANGE, "2");//lz1104
    int result = -1;
    // 保存XML文档到文件
    if (m_doc.SaveFile(project) == tinyxml2::XML_SUCCESS) {
        result = 0;
    }
    return result;
}
