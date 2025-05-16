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

    void printDataType(const DataType *dataType, int currentOffset) {
        const std::string& type = dataType->type;

        switch (dataType->dataType) {
        case 0:
            printVariableDataType(dataType, currentOffset);
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
            auto *arr = static_cast<const ArrayDataType *>(dataType);
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
                if (currentOffset == 0 && offset != 0) {
                    int padsToAdd = it->first - currentOffset;
                    if (padsToAdd != 0) {
                        ArrayDataType _array("uint8_t", "unk_", padsToAdd);
                        printArrayDataType(&_array, numUnknowns++, currentOffset);
                        currentOffset += it->first - currentOffset;
                    }
                }

                printDataType(it->second, currentOffset);

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
