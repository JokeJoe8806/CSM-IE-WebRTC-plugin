#ifndef _JSOBJECT_H_
#define _JSOBJECT_H_

#include <atlctl.h>
#include <atlcomcli.h>
#include <OleAuto.h>
#include <comutil.h>
using namespace ATL;

#include <vector>
#include <string>

inline int64_t GetInt(VARIANT* variant, int64_t default)
{
  switch (variant->vt)
  {
    case VT_I1:
      return V_I1(variant);
    case VT_I2:
      return V_I2(variant);
    case VT_I4:
      return V_I4(variant);
    case VT_UI1:
      return V_UI1(variant);
    case VT_UI2:
      return V_UI2(variant);
    case VT_INT:
      return V_INT(variant);
    case VT_UI4:
      return V_UI4(variant);
    case VT_UINT:
      return V_UINT(variant);
    case VT_I8:
      return V_I8(variant);
    case VT_UI8:
      return V_UI8(variant);
  }
  return default;
}


inline double GetDouble(VARIANT* variant, int64_t default)
{
  switch (variant->vt)
  {
    case VT_I1:
      return V_I1(variant);
    case VT_I2:
      return V_I2(variant);
    case VT_I4:
      return V_I4(variant);
    case VT_UI1:
      return V_UI1(variant);
    case VT_UI2:
      return V_UI2(variant);
    case VT_INT:
      return V_INT(variant);
    case VT_UI4:
      return V_UI4(variant);
    case VT_UINT:
      return V_UINT(variant);
    case VT_I8:
      return V_I8(variant);
    case VT_UI8:
      return V_UI8(variant);
    case VT_R4:
      return V_R4(variant);
    case VT_R8:
      return V_R8(variant);
  }
  return default;
}


class JSObject
{
public:
  JSObject(VARIANT& obj)
    : dispatchEx(V_DISPATCH(&obj))
  {

  }
  bool HasProperty(const std::wstring& name) const
  {
    HRESULT hr = E_NOTIMPL;
    DISPID dispId = DISPID_UNKNOWN;
    CComVariant result;
    CComExcepInfo exceptionInfo;
    DISPPARAMS params = { 0 };

    hr = dispatchEx->GetDispID(CComBSTR(name.c_str()), fdexNameEnsure | fdexNameCaseSensitive | 0x10000000, &dispId);

    return dispId!= DISPID_UNKNOWN;
  }

  bool HasNotNullProperty(const std::wstring& name) const
  {
    HRESULT hr = E_NOTIMPL;
    DISPID dispId = DISPID_UNKNOWN;
    CComVariant result;
    CComExcepInfo exceptionInfo;
    DISPPARAMS params = { 0 };

    hr = dispatchEx->GetDispID(CComBSTR(name.c_str()), fdexNameEnsure | fdexNameCaseSensitive | 0x10000000, &dispId);
    if (SUCCEEDED(hr) && dispId != DISPID_UNKNOWN)
      hr = dispatchEx->InvokeEx(dispId, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &result, &exceptionInfo, NULL);

    return result.vt>1;
  }


  CComVariant GetProperty(const std::wstring& name) {

    HRESULT hr = E_NOTIMPL;
    DISPID dispId = DISPID_UNKNOWN;
    CComVariant result;
    CComExcepInfo exceptionInfo;
    DISPPARAMS params = { 0 };

    hr = dispatchEx->GetDispID(CComBSTR(name.c_str()), fdexNameEnsure | fdexNameCaseSensitive | 0x10000000, &dispId);
    if (SUCCEEDED(hr) && dispId != DISPID_UNKNOWN)
      hr = dispatchEx->InvokeEx(dispId, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &result, &exceptionInfo, NULL);

    return result;
  }

  _bstr_t GetStringProperty(const std::wstring& name, const std::string& default = "") {
    //Get property
    auto prop = GetProperty(name);
    //check type
    if (prop.bstrVal)
      return prop;
    //Return empy
    return _bstr_t(default.c_str());
  }

  int64_t GetIntegerProperty(const std::wstring& name, int64_t default = 0) {
    auto prop = GetProperty(name);
    //Get property
    return GetInt(&prop, default);
  }

  double GetDoubleProperty(const std::wstring& name, int64_t default = 0)
  {
    auto prop = GetProperty(name);
    //Get property
    return GetDouble(&prop, default);
  }

  bool GetBooleanProperty(const std::wstring& name, bool default = false) {
    auto prop = GetProperty(name);
    //Get property
    if (prop.vt == VT_BOOL) 
      return prop.boolVal == VARIANT_TRUE;
    return GetInt(&prop, default);
  }

  std::vector<std::wstring> GetPropertyNames() {

    std::vector<std::wstring> names;

    DISPID dispid = DISPID_STARTENUM;
    while (dispatchEx->GetNextDispID(fdexEnumAll, dispid, &dispid) != S_FALSE) 
    {
      if (dispid < 0) 
        continue;
      CComBSTR memberName;
      if (SUCCEEDED(dispatchEx->GetMemberName(dispid, &memberName)))
        names.push_back(std::wstring(memberName));
    }
    return names;
  }

  JSObject GetObjectProperty(const std::wstring& name)
  {
    return std::move(JSObject(GetProperty(name)));
  }

  std::vector<JSObject> GetPropertyObjects()
  {

    std::vector<JSObject> objects;

    DISPID dispId = DISPID_STARTENUM;
    while (dispatchEx->GetNextDispID(fdexEnumAll, dispId, &dispId) != S_FALSE)
    {
      if (dispId < 0)
        continue;
      CComBSTR memberName;
      CComVariant result;
      CComExcepInfo exceptionInfo;
      DISPPARAMS params = { 0 };
      if (SUCCEEDED(dispatchEx->InvokeEx(dispId, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &result, &exceptionInfo, NULL)))
          objects.emplace_back(result);
    }
    return std::move(objects);
  }

  std::vector<CComVariant> GetArrayProperty(const std::wstring& name)
  {
    std::vector<CComVariant> variants;
    //Get property for name
    auto prop = GetProperty(name);

    //Check type
    if (prop.vt == VT_DISPATCH)
    {

      //Get js object
      JSObject array(prop);

      //If got it
      if (!array.isNull())
      {
        //Get length
        int length = array.GetIntegerProperty(L"length", 0);
        //Get all 
        for (int i = 0; i < length; ++i)
        {
          //Get stream
          auto element = array.GetProperty(std::to_wstring(i));
          //Add it
          variants.emplace_back(element);
        }
      }
    }
    return  std::move(variants);
  }

  std::vector<JSObject> GetArrayObjectProperty(const std::wstring& name)
  {
    std::vector<JSObject> objects;
    //Get property for name
    auto prop = GetProperty(name);

    //Check type
    if (prop.vt == VT_DISPATCH)
    {

      //Get js object
      JSObject array(prop);

      //If got it
      if (!array.isNull())
      {
        //Get length
        int length = array.GetIntegerProperty(L"length", 0);
        //Get all 
        for (int i = 0; i < length; ++i)
        {
          //Get stream
          auto element = array.GetProperty(std::to_wstring(i));
          //Add it
          objects.emplace_back(element);
        }
      }
    }
    return  std::move(objects);
  }

  bool isNull() {
    return !dispatchEx;
  }

private:
  CComQIPtr<IDispatchEx> dispatchEx;
};

#endif
