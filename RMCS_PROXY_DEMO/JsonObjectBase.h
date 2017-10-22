#ifndef CJSONOBJECTBASE_H  
#define CJSONOBJECTBASE_H  
#include <string>    
#include <vector>    
#include <list>  
#include "include/json/json.h"
#include "include/json/value.h"
using std::string;    
using std::vector;    
using std::list;  


#define DoArrayDeSerialize_Wrapper(listType, type) DoArrayDeSerialize<##listType<##type>, ##type>  

#define Type_Wrapper(listType, type) (##listType<##type>*)  

#define DoArraySerialize_Wrapper(listType, type) DoArraySerialize(Type_Wrapper(listType, type)addr);  

#define DoObjArrayDeSerialize_Wrapper(listType, type) DoObjArrayDeSerialize<##listType<type>>  

typedef  void* (*GenerateJsonObjForDeSerializeFromOutSide)(const string& propertyName);  
typedef  int INT;
typedef  unsigned int UINT;
typedef  long long  LONGLONG;
typedef  unsigned long long ULONGLONG;
typedef  float FLOAT;
typedef double DOUBLE;
struct  CJsonObjectBase
{    
protected:    
	enum CEnumJsonTypeMap    
	{  
		asBool = 1,    
		asInt,    
		asUInt,    
		asString,    
		asInt64,    
		asUInt64,    
		asJsonObj,  
		asSpecialArray,  
		asVectorArray,  
		asListArray, 
		asFloat,
		asDouble,
	};    
public:    
	CJsonObjectBase(void){}    
public:    
	virtual ~CJsonObjectBase(void){       
	}    
	string Serialize()    
	{    
		Json::Value new_item = DoSerialize();           
		Json::FastWriter writer;      
		std::string out2 = writer.write(new_item);     
		return out2;    
	}    

	bool DeSerialize(const char* str)    
	{    
		Json::Reader reader;      
		Json::Value root;    
		if (reader.parse(str, root))    
		{      
			return DoDeSerialize(root);    
		}    
		return false;    
	}    

	Json::Value DoSerialize()    
	{    
		Json::Value new_item;    
		int nSize = m_vectorName.size();    
		for (int i=0; i < nSize; ++i )    
		{    
			void* pAddr = m_vectorPropertyAddr[i];    
			switch (m_vectorType[i])
			{
			case asVectorArray:{
			
					//一般的非结构
					new_item[m_vectorName[i]] = DoArraySerialize(pAddr, m_vectorType[i], m_vectorListParamType[i]);

				
				break;
			}
			case asListArray:
				new_item[m_vectorName[i]] = DoArraySerialize(pAddr, m_vectorType[i], m_vectorListParamType[i]);
				break;
			case asSpecialArray:
				new_item[m_vectorName[i]] = DoSpecialArraySerialize(m_vectorName[i]);
				break;
			case asJsonObj:
				new_item[m_vectorName[i]] = ((CJsonObjectBase*)pAddr)->DoSerialize();
				break;
			case asBool:
				new_item[m_vectorName[i]] = Serialize<bool>(pAddr);
			case asInt:
				new_item[m_vectorName[i]] = Serialize<int>(pAddr);
				break;
			case asUInt:
				new_item[m_vectorName[i]] = Serialize<unsigned int>(pAddr);
				break;
			case asInt64:
				new_item[m_vectorName[i]] = Serialize<long long>(pAddr);
				break;
			case asUInt64:
				new_item[m_vectorName[i]] = Serialize<unsigned long long>(pAddr);
				break;
			case asString:{
				
				new_item[m_vectorName[i]] = Serialize<string>(pAddr);
				break;
			}
			case asFloat:    
				new_item[m_vectorName[i]] = Serialize<float>(pAddr); 
				break;
			case asDouble:    
				new_item[m_vectorName[i]] = Serialize<double>(pAddr); 
				break;
			default:    
				//我暂时只支持这几种类型，需要的可以自行添加  
				break;    
			}           
		}    
		return new_item;    
	}   
protected:  
	bool DoDeSerialize(Json::Value& root)    
	{    
		int nSize = m_vectorName.size();    
		for (int i=0; i < nSize; ++i )    
		{    
			void* pAddr = m_vectorPropertyAddr[i];    

			switch(m_vectorType[i])    
			{    
			case asListArray:  
			case asVectorArray:  
				{  
					if (root.isNull() || root[m_vectorName[i]].isNull())  
					{  
						break;  
					}  
					DoArrayDeSerialize(m_vectorName[i], pAddr, root[m_vectorName[i]], m_vectorType[i], m_vectorListParamType[i]);  
				}  
				break;  
			case asJsonObj:    
				{    
					if (!root[ m_vectorName[i] ].isNull())     
						((CJsonObjectBase*)pAddr)->DoDeSerialize(root[m_vectorName[i]]);    
				}                   
				break;    
			case asBool:    
				(*(bool*)pAddr) = root.get(m_vectorName[i], 0).asBool();    
				break;    
			case asInt:    
				(*(INT*)pAddr) = root.get(m_vectorName[i], 0).asInt();    
				break;    
			case asUInt:    
				(*(UINT*)pAddr) = root.get(m_vectorName[i], 0).asUInt();    
				break;    
			case asInt64:    
				(*(LONGLONG*)pAddr) = root.get(m_vectorName[i], 0).asInt64();    
				break;    
			case asUInt64:    
				(*(ULONGLONG*)pAddr) = root.get(m_vectorName[i], 0).asUInt64();    
				break;    
			case asString:    
				(*(string*)pAddr) = root.get(m_vectorName[i], "").asString();  
				break;
			case asFloat:
				(*(float*)pAddr) = root.get(m_vectorName[i], "").asFloat();
				break;
			case asDouble:
				(*(double*)pAddr) = root.get(m_vectorName[i], "").asDouble();
				break;
			default:    
				//我暂时只支持这几种类型，需要的可以自行添加     
				break;    
			}               
		}    
		return true;    
	}    

	void SetProperty(string name, CEnumJsonTypeMap type, void* addr,  CEnumJsonTypeMap listParamType = asInt)    
	{    
		m_vectorName.push_back(name);    
		m_vectorPropertyAddr.push_back(addr);    
		m_vectorType.push_back(type);    
		m_vectorListParamType.push_back(listParamType);  
	}    
	void clearProperty() {
		m_vectorName.clear();
		m_vectorPropertyAddr.clear();
		m_vectorType.clear();
		m_vectorListParamType.clear();
	
	}

	virtual void SetPropertys() = 0;    
	//有特殊对象需要序列化时，请重载此函数   
	virtual Json::Value DoSpecialArraySerialize(const string& propertyName){ return "";}  

	//在反序列化时，如果对象中嵌套了列表，并且列表中容纳的内容是其他特殊对象时，需要重载此函数，new出真正的对象  
	virtual CJsonObjectBase* GenerateJsonObjForDeSerialize(const string& propertyName){ return NULL;}  

	bool DoArrayDeSerialize(const string& propertyName, void* addr, Json::Value& root, CEnumJsonTypeMap listType, CEnumJsonTypeMap paramType)  
	{  
		if (listType == asVectorArray)  
		{  
			switch(paramType)  
			{   
			case asJsonObj:    
				{    
					return DoObjArrayDeSerialize_Wrapper(vector, CJsonObjectBase*)(propertyName, addr, root);  
				}                   
				break;    
			case asBool:    
				//无   
				break;   
			case asInt:    
				return DoArrayDeSerialize_Wrapper(vector, INT)(addr, root);   
			case asUInt:    
				return DoArrayDeSerialize_Wrapper(vector, UINT)(addr, root);     
			case asInt64:    
				return DoArrayDeSerialize_Wrapper(vector, LONGLONG)(addr, root);       
			case asUInt64:    
				return DoArrayDeSerialize_Wrapper(vector, ULONGLONG)(addr, root);       
			case asString:    
				return DoArrayDeSerialize_Wrapper(vector, string)(addr, root);  
			case asFloat:    
				return DoArrayDeSerialize_Wrapper(vector, float)(addr, root);
			case asDouble:    
				return DoArrayDeSerialize_Wrapper(vector, double)(addr, root);
			default:  
				break;    
			}  
		}  
		else if(listType == asListArray)  
		{  
			switch(paramType)  
			{   
			case asJsonObj:    
				{  
					return DoObjArrayDeSerialize_Wrapper(list, CJsonObjectBase*)(propertyName, addr, root);  
				}                   
				break;    
			case asBool:    
				return DoArrayDeSerialize_Wrapper(list, bool)(addr, root);  
			case asInt:    
				return DoArrayDeSerialize_Wrapper(list, INT)(addr, root);   
			case asUInt:    
				return DoArrayDeSerialize_Wrapper(list, UINT)(addr, root);     
			case asInt64:    
				return DoArrayDeSerialize_Wrapper(list, LONGLONG)(addr, root);       
			case asUInt64:    
				return DoArrayDeSerialize_Wrapper(list, ULONGLONG)(addr, root);       
			case asString:    
				return DoArrayDeSerialize_Wrapper(list, string)(addr, root);  
			case asFloat:    
				return DoArrayDeSerialize_Wrapper(list, float)(addr, root);  
			case asDouble:    
				return DoArrayDeSerialize_Wrapper(list, double)(addr, root); 
			default:  
				break;    
			}  
		}  

		return true;  
	}  

	//特殊对象列表的反序列化  
	template<typename T1>  
	bool DoObjArrayDeSerialize(const string& propertyName, void* addr, Json::Value& node)  
	{  
		if (!node.isArray())  
		{  
			return false;  
		}  
		T1* pList = (T1*)addr;  
		int size = node.size();  
		for(int i = 0; i < size; ++i)  
		{  
			CJsonObjectBase* pNode = GenerateJsonObjForDeSerialize(propertyName);  
			pNode->DoDeSerialize(node[i]);  
			pList->push_back(pNode);  
		}  
		return true;  
	}  
public:  
	//常见类型列表的反序列化   
	template <typename T1, typename T2>  
	  bool DoArrayDeSerialize(void* addr, Json::Value& node)  
	{  
		if (!node.isArray())  
		{  
			return false;  
		}  
		T1* pList = (T1*)addr;  
		int size = node.size();  
		for(int i = 0; i < size; ++i)  
			pList->push_back( DeSerialize<T2>(node[i]) );  
		return true;  
	}  

	//特殊对象列表的反序列化  
	template<typename T1>  
	  bool DoObjArrayDeSerialize(const string& propertyName, void* addr, Json::Value& node, GenerateJsonObjForDeSerializeFromOutSide funGenerate)  
	{  
		if (!node.isArray())  
		{  
			return false;  
		}  
		T1* pList = (T1*)addr;  
		int size = node.size();  
		for(int i = 0; i < size; ++i)  
		{  
			CJsonObjectBase* pNode = (CJsonObjectBase*)funGenerate(propertyName);  
			pNode->DoDeSerialize(node[i]);  
			pList->push_back(pNode);  
		}  
		return true;  
	}  
protected:  
	Json::Value DoArraySerialize(void* addr, CEnumJsonTypeMap listType, CEnumJsonTypeMap paramType)  
	{  
		if (listType == asVectorArray)  
		{  
			switch(paramType)  
			{   
			case asBool:    
				return "";  
			case asJsonObj:  
				return DoArraySerialize_Wrapper(vector, CJsonObjectBase*);
			case asInt:    
				return DoArraySerialize_Wrapper(vector, INT);  
			case asUInt:    
				return DoArraySerialize_Wrapper(vector, UINT);   
			case asInt64:    
				return DoArraySerialize_Wrapper(vector, LONGLONG);     
			case asUInt64:    
				return DoArraySerialize_Wrapper(vector, ULONGLONG);    
			case asString:    
				return DoArraySerialize_Wrapper(vector, string);  
			case asFloat:    
				return DoArraySerialize_Wrapper(vector, float);  
			case asDouble:    
				return DoArraySerialize_Wrapper(vector, double); 
			}  
		}  
		else   
		{  
			switch(paramType)  
			{   
			case asBool:    
				return DoArraySerialize_Wrapper(list, bool);  
			case asJsonObj:  
				return DoArraySerialize_Wrapper(list, CJsonObjectBase*);  
			case asInt:    
				return DoArraySerialize_Wrapper(list, INT);  
			case asUInt:    
				return DoArraySerialize_Wrapper(list, UINT);   
			case asInt64:    
				return DoArraySerialize_Wrapper(list, LONGLONG);     
			case asUInt64:    
				return DoArraySerialize_Wrapper(list, ULONGLONG);    
			case asString:    
				return DoArraySerialize_Wrapper(list, string);  
			case asFloat:    
				return DoArraySerialize_Wrapper(list, float);  
			case asDouble:    
				return DoArraySerialize_Wrapper(list, double);  
			}  
		}  
		return "";  
	}  
public:  
	template <typename T1>  
	  Json::Value DoArraySerialize(T1* pList)  
	{  
		Json::Value arrayValue;  
		for (T1::iterator it = pList->begin(); it != pList->end(); ++ it)  
		{  
			arrayValue.append(*it);           
		}  
		return arrayValue;  
	}     

	template <>  
	  Json::Value DoArraySerialize(vector<CJsonObjectBase*>* pList)  
	{  
		Json::Value arrayValue;  
		if(pList->size()<=0)return arrayValue;
		for (vector<CJsonObjectBase*>::iterator it = pList->begin(); it != pList->end(); ++ it)  
		{  
			arrayValue.append((*it)->DoSerialize());           
		}  
		return arrayValue;  
	}  
	  Json::Value DoArraySerialize_1(vector<CJsonObjectBase*>* pList)
	{
		Json::Value arrayValue;
		if (pList->size() <= 0)return arrayValue;
		for (vector<CJsonObjectBase*>::iterator it = pList->begin(); it != pList->end(); ++it)
		{
			arrayValue.append((*it)->DoSerialize());
		}
		return arrayValue;
	}
	template <>  
	  Json::Value DoArraySerialize(list<CJsonObjectBase*>* pList)  
	{  
		Json::Value arrayValue;  
		for (list<CJsonObjectBase*>::iterator it = pList->begin(); it != pList->end(); ++ it)  
		{  
			arrayValue.append((*it)->DoSerialize());           
		}  
		return arrayValue;  
	}  
	  string JsonValueToString(Json::Value& tvalue)  
	{  
		Json::FastWriter writer;      
		return writer.write(tvalue);  
	}  
private:  
	template <typename T>  
	Json::Value Serialize(void* addr)  
	{
		return (*(T*)addr);  
	}  
	template <>
	Json::Value Serialize<string>(void* addr)
	{
		string* a = (string*)addr;
		return a->data();
	}
	template <typename T>  
	  T DeSerialize(Json::Value& root)  
	{  
		return GetData<T>(root);  
	}  

	template <typename T>  
	  T GetData(Json::Value& root)  
	{  
		T temp;  
		return temp;  
	}  

	template <>  
	  bool GetData(Json::Value& root)  
	{  
		return root.asBool();  
	}  

	template <>  
	  INT GetData(Json::Value& root)  
	{  
		return root.asInt();  
	}  

	template <>  
	  UINT GetData(Json::Value& root)  
	{  
		return root.asUInt();  
	}  
	template <>  
	  LONGLONG GetData(Json::Value& root)  
	{  
		return root.asInt64();  
	}  
	template <>  
	  ULONGLONG GetData(Json::Value& root)  
	{  
		return root.asUInt64();  
	}  
	template <>  
	  string GetData(Json::Value& root)  
	{  
		return root.asString();  
	}  
	template <>  
	  float GetData(Json::Value& root)  
	{  
		return root.asFloat();  
	}
	template <>  
	  double GetData(Json::Value& root)  
	{  
		return root.asDouble();  
	}
private:    
	vector<string> m_vectorName;    
	vector<void*>  m_vectorPropertyAddr;    
	vector<CEnumJsonTypeMap>     m_vectorType;  
	vector<CEnumJsonTypeMap>     m_vectorListParamType;  
};   
#endif  