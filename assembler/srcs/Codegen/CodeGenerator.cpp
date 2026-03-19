#include<cstdint>
#include<vector>
#include<fstream>
#include<map>

#include "../../headers/Codegen/CodeGenerator.hpp"
#include "../../headers/Parser/Parser.hpp"
#include "../../headers/LexicalAnalyzer/Lexer.hpp"

std::ofstream CodeGenerator::makeBinaryFile(std::string outputFilePath, ParseResult* parseResult) {
    std::ofstream outputfile(outputFilePath, std::ios::out);

    std::vector<ProgIns*> instructions = parseResult->program_instructions;
    for(int i = 0; i < instructions.size(); i++) {

        bin_form* bin = instructionToByteCode(instructions[i], parseResult->symmap);
        printf("%s -> ", instructions[i]->toString().c_str());
        for(int j = 0; j < bin->size; j++) {
            printf("0x%02x ", bin->bindata[j]);
        }
        printf("\n");
        outputfile.seekp(instructions[i]->addr, std::ios::beg);
        outputfile.write(reinterpret_cast<const char*>(bin->bindata), bin->size);
        free(bin->bindata);
        free(bin);
    }

    std::vector<ProgData*> data = parseResult->program_data;
    
    if (data.size() != 0) {
        for(int i = 0; i < data.size(); i++) {

            bin_form* bin = dataToBin(data[i]);
            outputfile.seekp(data[i]->addr, std::ios::beg);
            outputfile.write(reinterpret_cast<const char*>(bin->bindata), bin->size);
            free(bin->bindata);
            free(bin);
        }
    }
    
    // write metadata;
    
    file_metadata filemeta = {0};
    filemeta.signature = signature.c_str();
    filemeta.date = date.c_str();
    filemeta.ver = version;
    filemeta.arch = 32;

    try {
        uint32_t entry = parseResult->symmap.at("MAIN");
        filemeta.entrypoint = entry;
    }
    catch(const std::out_of_range& e) {
        std::cout << 
        "Warning: MAIN not found. setting the entry point value to the beginning of CODE section instead(0 if also not found)"
        << std::endl;
        try {
            uint32_t entry = parseResult->sections.at("CODE");
            filemeta.entrypoint = entry;
        } catch(const std::out_of_range& e) {
            filemeta.entrypoint = 0;
        }
    }

    outputfile.seekp(0, std::ios::end);

    outputfile.write(reinterpret_cast<const char*>(filemeta.signature), siglen);
    outputfile.write(reinterpret_cast<const char*>(filemeta.date), datelen);
    outputfile.write(reinterpret_cast<const char*>(&filemeta.ver), sizeof(uint16_t));
    outputfile.write(reinterpret_cast<const char*>(&filemeta.arch), sizeof(uint8_t));
    outputfile.write(reinterpret_cast<const char*>(&filemeta.entrypoint), sizeof(uint32_t));

    return outputfile;
}

bin_form* CodeGenerator::instructionToByteCode(ProgIns* ins) {

    bin_form* f = (bin_form*) malloc(sizeof(bin_form));
    f->size = ins->len;
    f->bindata = (uint8_t*) calloc(f->size, sizeof(uint8_t));
    
    int index = 0;
    f->bindata[index++] = ins->encoding_info->opcode;
    
    if (!ins->hasOperands) return f;
    
    else {
        f->bindata[index++] = ins->encoding_info->operand_types;

        if (ins->encoding_info->register_select != REGSELECT_DEFAULT) f->bindata[index++] = ins->encoding_info->register_select;

        if (ins->hasMemory){

            f->bindata[index++] = ins->encoding_info->displacement_info;

            if (ins->hasDisplacement){
                std::vector<uint8_t> displacement = dwordToBytes(ins->encoding_info->disp_val);
                for(int i = 0; i < displacement.size(); i++) f->bindata[index++] = displacement[i];
            }
        }

        if (ins->hasImmediate) {

            if (ins->immediateSize == 1) f->bindata[index++] = (uint8_t) ins->encoding_info->imm_val;

            else if (ins->immediateSize == 2) {
                std::vector<uint8_t> imm = wordToBytes(ins->encoding_info->imm_val);
                for(int i = 0; i < imm.size(); i++) f->bindata[index++] = imm[i];
            }
            else if (ins->immediateSize == 4) {
                std::vector<uint8_t> imm = dwordToBytes(ins->encoding_info->imm_val);
                for(int i = 0; i < imm.size(); i++) f->bindata[index++] = imm[i];
            }
        }
    }

    return f;
}

bin_form* CodeGenerator::instructionToByteCode(ProgIns* ins, std::map<std::string, uint32_t> symmap) {
    bin_form* f = (bin_form*) malloc(sizeof(bin_form));
    f->size = ins->len;
    f->bindata = (uint8_t*) calloc(f->size, sizeof(uint8_t));
    
    int index = 0;
    f->bindata[index++] = ins->encoding_info->opcode;
    
    if (!ins->hasOperands) return f;
    
    else {
        if(ins->encoding_info->operand_types != OPERTYPE_DEFAULT) f->bindata[index++] = ins->encoding_info->operand_types;

        if (ins->encoding_info->register_select != REGSELECT_DEFAULT) f->bindata[index++] = ins->encoding_info->register_select;

        if (ins->hasMemory){

            f->bindata[index++] = ins->encoding_info->displacement_info;

            if (ins->hasDisplacement) {
                std::vector<uint8_t> displacement = dwordToBytes(ins->encoding_info->disp_val);
                for(int i = 0; i < displacement.size(); i++) f->bindata[index++] = displacement[i];
            }
        }

        if (ins->hasImmediate) {

            if (ins->immediateSize == 1) f->bindata[index++] = (uint8_t) ins->encoding_info->imm_val;

            else if (ins->immediateSize == 2) {
                std::vector<uint8_t> imm = wordToBytes(ins->encoding_info->imm_val);
                for(int i = 0; i < imm.size(); i++) f->bindata[index++] = imm[i];
            }
            else if (ins->immediateSize == 4) {
                std::vector<uint8_t> imm = dwordToBytes(ins->encoding_info->imm_val);
                for(int i = 0; i < imm.size(); i++) f->bindata[index++] = imm[i];
            }
        }

        if (ins->hasSymbol) {
            uint32_t val = symmap[ins->symbol];
            std::vector<uint8_t> symval = dwordToBytes(val);
            for(int i = 0; i < symval.size(); i++){
                f->bindata[index++] = symval[i];
            }
        }
    }

    return f;
}

// This function doesn't handle resb or resw directives.
// They're handled at makeBinaryFile() function.
// since they just tell the assembler to skip over a given number of bytes and no data is generated.
bin_form* CodeGenerator::dataToBin(ProgData* data) {
    bin_form* f = (bin_form*) malloc(sizeof(bin_form));
    f->size = data->len;
    f->bindata = (uint8_t*) calloc(f->size, sizeof(uint8_t)); 

    int index = 0;
    int mode = data->data_store_mode;

    if (data->isConst) {
        std::vector<uint8_t> d = dwordToBytes(data->constval);
        f->bindata[index++] = d[0];
        f->bindata[index++] = d[1];
        f->bindata[index++] = d[2];
        f->bindata[index++] = d[3];
    }
    else if (data->parts[1].subtype == SubType::DIR_RESB || data->parts[1].subtype == SubType::DIR_RESW) {

        f->size = (uint8_t) std::stoi(data->parts[2].tokenstr);
        f->bindata = (uint8_t*) calloc(f->size, sizeof(uint8_t));
    }
    else{
        for(int i = 0; i < data->parts.size(); i++) {

            switch(data->parts[i].maintype) {

                case MainType::NUM: {
                        uint32_t val = (uint32_t) std::stoi(data->parts[i].tokenstr);
                        if (mode == MODE_BYTE) f->bindata[index++] = (uint8_t) val;
                        else if (mode == MODE_WORD) {
                            std::vector<uint8_t> d = wordToBytes(val);
                            f->bindata[index++] = d[0];
                            f->bindata[index++] = d[1];
                        }
                        else if (mode == MODE_DWORD) {
                            std::vector<uint8_t> d = dwordToBytes(val);
                            f->bindata[index++] = d[0];
                            f->bindata[index++] = d[1];
                            f->bindata[index++] = d[2];
                            f->bindata[index++] = d[3];
                        }
                    break;
                }
                case MainType::STR: {
                        std::string s = data->parts[i].tokenstr;
                        if (mode == MODE_BYTE) {
                            for(int i = 0; i < s.length(); i++) f->bindata[index++] = (uint8_t) s[i];
                        }
                        else if (mode == MODE_WORD) {
                            for(int i = 0; i < s.length(); i++) {
                                std::vector<uint8_t> d = wordToBytes( (uint16_t) s[i]);
                                f->bindata[index++] = d[0];
                                f->bindata[index++] = d[1];
                            }
                        }
                        else if (mode == MODE_DWORD) {
                            for(int i = 0; i < s.length(); i++) {
                                std::vector<uint8_t> d = dwordToBytes( (uint32_t) s[i]);
                                f->bindata[index++] = d[0];
                                f->bindata[index++] = d[1];
                                f->bindata[index++] = d[2];
                                f->bindata[index++] = d[3];
                            }
                        }
                    break;
                }
                default:
                break;
            }
        }
    }
    return f;
}

std::vector<uint8_t> CodeGenerator::dwordToBytes(uint32_t val) {
    std::vector<uint8_t> d;
    d.push_back( (val >> 24) & 0xff );
    d.push_back( (val >> 16) & 0xff );
    d.push_back( (val >> 8) & 0xff );
    d.push_back(val & 0xff);

    return d;
}

std::vector<uint8_t> CodeGenerator::wordToBytes(uint16_t val) {
    std::vector<uint8_t> d;
    d.push_back( (val >> 8) & 0xff );
    d.push_back(val & 0xff);
    return d;
}