#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

struct DataType {
    int dataType;
    std::string type;
    std::string name;
    DataType(const std::string& type, const std::string& name, int dataType = 0) : type(type), name(name), dataType(dataType) {}
};

struct ArrayDataType : public DataType {
    int size;
    ArrayDataType(const std::string& type, const std::string& name, int size) : DataType(type, name, 1), size(size) {}
};

struct StructureCreator {
private:
    std::vector<std::pair<int, DataType *>> dataTypes;
    int packedSize;
    int size;
    std::string compiler;
public:
    StructureCreator(const std::string& compiler) : packedSize(4), size(4), compiler(compiler) {}
    ~StructureCreator() {
        for (auto& i : dataTypes)
            delete i.second;
        dataTypes.clear();
    }

    void setPackingSize(int bytes) {
        packedSize = bytes;
    }

    void setSize(int size) {
        this->size = size;
    }

    void addDataType(int offset, DataType *type) {
        dataTypes.push_back({ offset, type });    
    }

    void printVariableDataType(const DataType *dataType, int offset = -1) {
        printf("    %s %s;", dataType->type.c_str(), dataType->name.c_str());
        if (offset != -1)
            printf(" // 0x%x", offset);
        printf("\n");
    }

    void printArrayDataType(const ArrayDataType *dataType, int count = -1, int offset = -1) {
        printf("    %s %s", dataType->type.c_str(), dataType->name.c_str());
        if (count != -1) {
            printf("%d", count);
        }
        printf("[0x%x];", dataType->size);
        if (offset != -1)
            printf(" // 0x%x", offset);
        printf("\n");
    }

    void printStruct(const char *structName = "anonymous") {
        int currentOffset = 0;
        int numUnknowns = 0;

        std::sort(dataTypes.begin(), dataTypes.end(), [](const std::pair<int, DataType *>& a, const std::pair<int, DataType *>& b) { return a.first < b.first; });

        if (compiler == "msvc") {
            printf("#pragma pack(push, %d)\n\n", packedSize);
            printf("struct %s {\n", structName);
        } else if (compiler == "gcc") {
            printf("struct __attribute__((packed, aligned(%d)) %s {\n", packedSize, structName);
        }

        for (auto it = dataTypes.begin(); it != dataTypes.end(); it++) {
            int offset = it->first;
            if (currentOffset <= offset) {
                const std::string& type = it->second->type;

                switch (it->second->dataType) {
                case 0:
                    printVariableDataType(it->second, currentOffset);
                    if (type == "uint8_t") {
                        currentOffset += 1;
                    } else if (type == "uint16_t") {
                        currentOffset += 2;
                    } else if (type == "uint32_t") {
                        currentOffset += 4;
                    } else if (type == "uint64_t") {
                        currentOffset += 8;
                    }
                    break;
                case 1:
                {
                    auto *arr = static_cast<ArrayDataType *>(it->second);
                    printArrayDataType(arr, -1, currentOffset);
                    if (type == "uint8_t") {
                        currentOffset += 1 * arr->size;
                    } else if (type == "uint16_t") {
                        currentOffset += 2 * arr->size;
                    } else if (type == "uint32_t") {
                        currentOffset += 4 * arr->size;
                    } else if (type == "uint64_t") {
                        currentOffset += 8 * arr->size;
                    }
                    break;
                }
                }

                if (auto it2 = it + 1; it2 != dataTypes.end()) {
                    if (it2->first >= currentOffset) {
                        int padsToAdd = it2->first - currentOffset;
                        if (padsToAdd != 0) {
                            ArrayDataType _array("uint8_t", "unk_", padsToAdd);
                            printArrayDataType(&_array, numUnknowns++, currentOffset);
                            currentOffset += it2->first - currentOffset;
                        }
                    } else {
                        printf("INVALID STRUCTURE FIELD %s", it->second->name.c_str());
                        std::exit(0);
                    }
                }

            }
        }
        
        if (currentOffset < size) {
            ArrayDataType _array("uint8_t", "unk_", size - currentOffset);
            printArrayDataType(&_array, numUnknowns, currentOffset);
            currentOffset += size - currentOffset;
            printf("    /* ^--- remove pad if needed */\n");
        }

        printf("};\n");
        if (compiler == "msvc")
            printf("#pragma pack(pop)\n");
        assert(currentOffset == size);
    }
};

#pragma pack(push, 1)
struct anonymous {
    uint8_t a; // 0x0
    uint32_t b; // 0x1
    uint8_t unk0[2]; // 0x5
    uint8_t c; // 0x7
    uint32_t d; // 0x8
};
#pragma pack(pop)

int main(int argc, char *argv[]) {
    StructureCreator creator { "msvc" };
    creator.setPackingSize(4);
    creator.setSize(0x400);

    creator.addDataType(0, new DataType("uint8_t", "hello", 0));
    creator.addDataType(0x10, new ArrayDataType("uint32_t", "gello", 20));
    creator.printStruct("mystruct");
    return 0;
}