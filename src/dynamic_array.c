#include <dynamic_array.h>


void init_array(DynamicArray* dynamicArray) {
    dynamicArray->data = (char*)malloc(1);
    if (dynamicArray->data != NULL) {
        dynamicArray->data[0] = '\0';
    }
    else {
        printf("Allocation failed\n");
    }

}

void push_back_item(DynamicArray* dynamicArray, const char* newChar) {
    int length = strlen(dynamicArray->data);
    int newLength = strlen(newChar);

    char* temp = (char*)realloc(dynamicArray->data, length + newLength + 1); // +1 für den Nullterminator
    if (temp == NULL) {
        printf("Reallocation failed\n");
        return;
    }

    dynamicArray->data = temp;
    strncpy(dynamicArray->data + length, newChar, newLength);
    dynamicArray->data[length + newLength] = '\0'; // Manuelles Nullterminieren nicht vergessen!

}

void delete_item(DynamicArray* dynamicArray, int index) {
    int length = strlen(dynamicArray->data);

    if (length == 0) {
        printf("Array is empty!\n");
        return;
    }

    // Falls index -1 ist, lösche einfach das letzte Zeichen
    if (index == -1) {
        index = length - 1;
    }

    // Falls Index außerhalb des gültigen Bereichs liegt
    if (index < 0 || index >= length) {
        printf("Index out of bound!\n");
        return;
    }

    // Falls nicht -1, Elemente nach links schieben
    if (index != length - 1) {
        for (int i = index; i < length - 1; i++) {
            dynamicArray->data[i] = dynamicArray->data[i + 1];
        }
    }

    // Speicher anpassen
    char* temp = (char*)realloc(dynamicArray->data, length);
    if (!temp) {
        printf("Deletion failed!\n");
        return;
    }

    dynamicArray->data = temp;
    dynamicArray->data[length - 1] = '\0';
}


void free_array(DynamicArray* dynamicArray) {
    free(dynamicArray->data);
}