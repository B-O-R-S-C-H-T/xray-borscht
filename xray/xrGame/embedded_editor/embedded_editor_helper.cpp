#include "pch_script.h"
#include "embedded_editor_helper.h"
#include <luabind/class_info.hpp>

xr_string to_string(const luabind::object& o, xr_string offset)
{
    int type = o.type();
    xr_string s;
    switch (type) {
    case LUA_TNIL:
        s = "nil";
        break;
    case LUA_TBOOLEAN:
        s = luabind::object_cast<bool>(o) ? "true" : "false";
        break;
    case LUA_TNUMBER: {
        std::string temp = std::to_string(luabind::object_cast<float>(o));
        std::string::size_type n = temp.find('.');
        if (n != std::string::npos) {
            bool isZ = true;
            for (std::string::size_type i = n + 1; i != temp.size(); ++i)
                if (temp[i] != '0')
                    isZ = false;
            if (isZ)
                temp[n] = '\0';
        }
        s = temp.c_str();
    } break;
    case LUA_TSTRING:
        s = luabind::object_cast<LPCSTR>(o);
        break;
    case LUA_TTABLE: {
        s = "{\n";
        xr_string newOffset = offset + "  ";
        for (luabind::object::iterator i = o.begin(), e = o.end(); i != e; ++i)
            s += newOffset + to_string(i.key(), newOffset) + "=" + to_string(*i, newOffset) + "\n";
        s += offset + "}";
    } break;
    case LUA_TUSERDATA: {
        s = "<userdata=";
        auto info = luabind::get_class_info(o);
        s += info.name.c_str();
        s += ">";
    } break;
    default:
        s = "<type=";
        s += std::to_string(type).c_str();
        s += ">";
    }
    return s;
}

xr_string toUtf8(const char* s)
{
    static xr_vector<wchar_t> buf;
    int n = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
    buf.resize(n);
    MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], buf.size());
    xr_string result;
    n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), nullptr, 0, nullptr, nullptr);
    result.resize(n);
    n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), &result[0], result.size(), nullptr, nullptr);
    return result;
}

Fvector2 world_to_screen(const Fvector& world) {
	Fvector4	   clip{};
	const Fmatrix& transform = Device.mFullTransform;
	clip.x					 = world.x * transform._11 + world.y * transform._21 + world.z * transform._31 + transform._41;
	clip.y					 = world.x * transform._12 + world.y * transform._22 + world.z * transform._32 + transform._42;
	clip.z					 = world.x * transform._13 + world.y * transform._23 + world.z * transform._33 + transform._43;
	clip.w					 = world.x * transform._14 + world.y * transform._24 + world.z * transform._34 + transform._44;

	if (clip.w < EPS_L)
		return Fvector2{0.f, 0.f};

	float inv_w	   = 1.f / clip.w;
	float ndc_x	   = clip.x * inv_w;
	float ndc_y	   = clip.y * inv_w;

	float screen_x = (ndc_x + 1.f) * 0.5f * float(Device.dwWidth);
	float screen_y = (1.f - ndc_y) * 0.5f * float(Device.dwHeight);

	return Fvector2{screen_x, screen_y};
}

