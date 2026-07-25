#pragma once

/*!***************************************************************************
 * \file   ShaderHelper.h
 * \author Alif Bin Norazhar(alif.binnorazhar@digipen.edu)
 * \par    CSD2101
 * \date   2026
 * \brief  Reusable shader compilation helper.  Supports two loading methods:
 *         (1) inline source strings ("stringify"), and
 *         (2) external .glsl files read from disk (Desktop/Web) or APK assets
 *             (Android).  All paths funnel into the same compile/link core.
 *****************************************************************************/

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper
{
public:
    // -----------------------------------------------------------------------
    // Core compile / link  (shared by both loading methods)
    // -----------------------------------------------------------------------

    /*!*************************************************************************
     * \brief  Compile a single GLSL shader from a source string.
     * \param  type
     *         GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * \param  src
     *         Null-terminated GLSL source text.
     * \return Non-zero shader object on success, 0 on failure.
     **************************************************************************/
    static GLuint compileShader(GLenum type, const char* src)
    {
        GLuint shader = glCreateShader(type);
        if (!shader) { LOGE("glCreateShader failed (type=0x%x)", type); return 0; }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            LOGE("Shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    /*!*************************************************************************
     * \brief  Link a vertex and fragment shader into a program.
     * \param  vert
     *         Compiled vertex shader object.
     * \param  frag
     *         Compiled fragment shader object.
     * \return Non-zero program object on success, 0 on failure.
     **************************************************************************/
    static GLuint linkProgram(GLuint vert, GLuint frag)
    {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);
        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("Program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            prog = 0;
        }
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    // -----------------------------------------------------------------------
    // Method 1: stringify 
    // -----------------------------------------------------------------------

    /*!*************************************************************************
     * \brief  Compile and link a program from raw GLSL source strings.
     * \param  vertSrc
     *         Null-terminated vertex shader source.
     * \param  fragSrc
     *         Null-terminated fragment shader source.
     * \return Non-zero program object on success, 0 on failure.
     **************************************************************************/
    static GLuint buildProgram(const char* vertSrc, const char* fragSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER,   vertSrc);
        if (!vs) return 0;
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!fs) { glDeleteShader(vs); return 0; }
        return linkProgram(vs, fs);
    }

    // -----------------------------------------------------------------------
    // Method 2: file-based
    // -----------------------------------------------------------------------

#ifdef PLATFORM_ANDROID

    /*!*************************************************************************
     * \brief  Read an asset file out of the APK into a std::string.
     * \param  mgr
     *         AAssetManager pointer received from Java via nativeInit.
     * \param  path
     *         Asset-relative path, e.g. "shader/BlueTriangleVertex.glsl".
     * \return File contents, or empty string on failure.
     **************************************************************************/
    static std::string loadAsset(AAssetManager* mgr, const char* path)
    {
        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) { LOGE("ShaderHelper: cannot open asset '%s'", path); return {}; }
        size_t size = static_cast<size_t>(AAsset_getLength(asset));
        std::string src(size, '\0');
        AAsset_read(asset, &src[0], size);
        AAsset_close(asset);
        return src;
    }

    /*!*************************************************************************
     * \brief  Build a shader program by loading .glsl files from APK assets.
     * \param  mgr
     *         AAssetManager pointer received from Java via nativeInit.
     * \param  vertPath
     *         Asset-relative path to the vertex shader.
     * \param  fragPath
     *         Asset-relative path to the fragment shader.
     * \return Non-zero program object on success, 0 on failure.
     **************************************************************************/
    static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath)
    {
        std::string vs = loadAsset(mgr, vertPath);
        std::string fs = loadAsset(mgr, fragPath);
        if (vs.empty() || fs.empty()) return 0;
        return buildProgram(vs.c_str(), fs.c_str());
    }

#else

    /*!*************************************************************************
     * \brief  Read a .glsl file from disk, trying several relative paths.
     * \param  filename
     *         Bare filename, e.g. "BlueTriangleVertex.glsl".
     * \return File contents, or empty string if not found in any search path.
     **************************************************************************/
    static std::string loadFile(const char* filename)
    {
        const char* prefixes[] = 
        {
            "assets/shader/",
            "",
            "shader/",
            "assets/"
        };

        for (const char* prefix : prefixes)
        {
            std::string path = std::string(prefix) + filename;
            std::ifstream f(path);
            if (f.is_open())
            {
                std::ostringstream ss;
                ss << f.rdbuf();
                return ss.str();
            }
        }
        LOGE("ShaderHelper: cannot find '%s' in any search path", filename);
        return {};
    }

    /*!*************************************************************************
     * \brief  Build a shader program by loading .glsl files from disk.
     * \param  vertFile
     *         Bare filename of the vertex shader.
     * \param  fragFile
     *         Bare filename of the fragment shader.
     * \return Non-zero program object on success, 0 on failure.
     **************************************************************************/
    static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile)
    {
        std::string vs = loadFile(vertFile);
        std::string fs = loadFile(fragFile);
        if (vs.empty() || fs.empty()) return 0;
        return buildProgram(vs.c_str(), fs.c_str());
    }

#endif // PLATFORM_ANDROID
};