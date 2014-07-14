#include "hsail_c.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "HSAILBrigContainer.h"
#include "HSAILBrigObjectFile.h"
#include "HSAILParser.h"
#include "HSAILDisassembler.h"
#include "HSAILValidator.h"

using namespace HSAIL_ASM;

namespace {

struct Api {
    BrigContainer   container;
    std::string     errorText;

    Api()
    : container()
    , errorText() 
    {
    }

    Api(const char *string_data, size_t string_size,
          const char *directive_data, size_t directive_size,
          const char *inst_data, size_t inst_size,
          const char *operand_data, size_t operand_size,
          const char* debug_data, size_t debug_size)
    : container(
            (char*)string_data, string_size,
            (char*)directive_data, directive_size,
            (char*)inst_data, inst_size,
            (char*)operand_data, operand_size,
            (char*)debug_data, debug_size)
    , errorText()
    {
    }
};

static int assemble(brig_container_t handle, std::istream& is)
{
    try {
        Scanner s(is, true);
        Parser p(s, ((Api*)handle)->container);
        p.parseSource();
    }
    catch(const SyntaxError& e) {
        std::stringstream ss;
        e.print(ss, is);
        ((Api*)handle)->errorText = ss.str();
        return 1;
    }
    Validator v(((Api*)handle)->container);
    if (!v.validate(Validator::VM_BrigLinked, true)) {
        std::stringstream ss;
        ss << v.getErrorMsg(&is) << "\n";
        int rc = v.getErrorCode();
        ((Api*)handle)->errorText = ss.str();
        return rc;
    }
    return 0;
}

}

extern "C" {

HSAIL_C_API brig_container_t brig_container_create_empty() 
{
    return (brig_container_t)new Api();
}

HSAIL_C_API brig_container_t brig_container_create(const char *string_data, size_t string_size,
                            const char *directive_data, size_t directive_size,
                            const char *inst_data, size_t inst_size,
                            const char *operand_data, size_t operand_size,
                            const char* debug_data, size_t debug_size)
{
    return (brig_container_t)new Api(
            string_data, string_size,
            directive_data, directive_size,
            inst_data, inst_size,
            operand_data, operand_size,
            debug_data, debug_size);
}

HSAIL_C_API brig_container_t brig_container_create_copy(const char *string_data, size_t string_size,
                            const char *directive_data, size_t directive_size,
                            const char *inst_data, size_t inst_size,
                            const char *operand_data, size_t operand_size,
                            const char* debug_data, size_t debug_size)
{
  Api *api = new Api;
  api->container.strings().setData(string_data, string_size);
  api->container.directives().setData(directive_data, directive_size);
  api->container.code().setData(inst_data, inst_size);
  api->container.operands().setData(operand_data, operand_size);
  api->container.debugChunks().setData(debug_data, debug_size);
  return (brig_container_t)api;
}

HSAIL_C_API const char* brig_container_get_section_data(brig_container_t handle, int section_id)
{
    return ((Api*)handle)->container.sectionById(section_id).data().begin;
}

HSAIL_C_API size_t brig_container_get_section_size(brig_container_t handle, int section_id)
{
    return ((Api*)handle)->container.sectionById(section_id).size();
}

HSAIL_C_API int brig_container_assemble_from_memory(brig_container_t handle, const char* text, size_t text_length)
{
    std::string s((char*)text, text_length);
    std::istringstream is(s);
    return assemble(handle, is);
}

HSAIL_C_API int brig_container_assemble_from_file(brig_container_t handle, const char* filename)
{
    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    std::stringstream ss;
    if ((!ifs.is_open()) || ifs.bad()) {
        ss << "Could not open "<< filename;
        ((Api*)handle)->errorText = ss.str();
        return 1;
    }
    return assemble(handle, ifs);
}

HSAIL_C_API int brig_container_disassemble_to_file(brig_container_t handle, const char* filename)
{
    Disassembler d(((Api*)handle)->container);
    d.setOutputOptions(0);
    std::stringstream ss;
    d.log(ss);
    int rc = d.run(filename);
    ((Api*)handle)->errorText = ss.str();
    return rc;
}

HSAIL_C_API int brig_container_load_from_mem(brig_container_t handle, const char* buf, size_t buf_length)
{
    std::stringstream ss;
    int rc = BrigIO::load(((Api*)handle)->container, FILE_FORMAT_AUTO, BrigIO::memoryReadingAdapter(buf, buf_length, ss));
    ((Api*)handle)->errorText = ss.str();
    return rc;
}

HSAIL_C_API int brig_container_load_from_file(brig_container_t handle, const char* filename)
{
    std::stringstream ss;
    int rc = BrigIO::load(((Api*)handle)->container, FILE_FORMAT_AUTO, BrigIO::fileReadingAdapter(filename, ss));
    ((Api*)handle)->errorText = ss.str();
    return rc;
}

HSAIL_C_API int brig_container_save_to_file(brig_container_t handle, const char* filename)
{
    std::stringstream ss;
    int rc = BrigIO::save(((Api*)handle)->container, FILE_FORMAT_BRIG | FILE_FORMAT_ELF32, BrigIO::fileWritingAdapter(filename, ss));
    ((Api*)handle)->errorText = ss.str();
    return rc;
}

HSAIL_C_API int brig_container_validate(brig_container_t handle)
{
    std::stringstream ss;
    Validator v(((Api*)handle)->container);
    if (!v.validate(Validator::VM_BrigLinked, true)) {
        ss << v.getErrorMsg(0) << "\n";
        int rc = v.getErrorCode();
        ((Api*)handle)->errorText = ss.str();
        return rc;
    }
    return 0;
}

HSAIL_C_API const char* brig_container_get_error_text(brig_container_t handle) {
    return ((Api*)handle)->errorText.c_str();
}

HSAIL_C_API void brig_container_destroy(brig_container_t handle)
{
    delete (Api*)handle;
}

}
