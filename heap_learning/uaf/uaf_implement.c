#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char notes[0x20];
    void (*print)(void*);
} note;

note *notes_ptr[3];
int flag[3] = {0};

void print(note *notes) {
    printf("Your note is : %s\n", notes->notes);
}

void bc(){
    puts("You get the backdoor.");
    puts("Congratulations!!!!!");
    system("/bin/sh");
}

int init_notes(note **notes, char* input_notes) {
    *notes = malloc(sizeof(note));
    if (!*notes) {
        puts("Failed to allocate memory");
        return 0;
    }
    strncpy((*notes)->notes, input_notes, 0x1f);
    (*notes)->print = (void*)&print;
    return 1;
}

int create_notes() {
    int idx;
    char input_notes[0x20];
    puts("where do you want to create the notes?");
    scanf("%d", &idx);
    if (idx < 0 || idx >= 3) {
        puts("Invalid index");
        return 0;
    }
    puts("what information do you want to show?");
    read(0, input_notes, 0x20);
    init_notes(&notes_ptr[idx], input_notes);
    flag[idx] = 1;
    return 1;
}

int edit_notes() {
    int idx;
    printf("where you want to edit:");
    scanf("%d", &idx);
    if (flag[idx]) {
        puts("Sorry");
        puts("This notes still in use");
        puts("Please change your choice");
        return 0;
    }
    puts("what do you want to change?");
    read(0, notes_ptr[idx]->notes, 0x21);
    return 1;
}

int del_notes() {
    int idx;
    printf("where you want to delete:");
    scanf("%d", &idx);
    if (idx < 0 || idx >= 3) {
        puts("Invalid index");
        return 0;
    }
    if (!flag[idx]) {
        puts("You cannot free a note twice!!!!!");
        return 0;
    }
    flag[idx] = 0;
    free(notes_ptr[idx]);
    puts("freed");
    return 1;
}

int exec_notes() {
    int idx;
    printf("where do you want to know:");
    scanf("%d", &idx);
    if ((idx < 0 || idx >= 3) && !flag[idx]) {
        puts("Invalid index");
        return 0;
    }
    notes_ptr[idx]->print(notes_ptr[idx]);
    return 1;
}

void menu() {
    printf("\n----------\n");
    puts("1. create notes");
    puts("2. edit notes");
    puts("3. delete notes");
    puts("4. execute notes");
    puts("5. exit");
    printf("\n----------\n");
}

void init(){
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
}

int main(){
    init();
    puts("Hello, welcome to my uaf training.");
    puts("Let's start!!!!!!!!!!");
    int choice;
    while (1) {
        menu();
        printf("please input your choice:");
        scanf("%d", &choice);
        switch (choice) {
            case 1: create_notes(); break;
            case 2: edit_notes(); break;
            case 3: del_notes(); break;
            case 4: exec_notes(); break;
            case 5: exit(0);
            default: break;
        }
    }
    return 0;
}