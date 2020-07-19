#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define COMMAND_OK 0
#define COMMAND_ERR 1
#define NAME_LEN 32
#define NUM_LEN 32
#define NAME_INPUT "%32s"
#define NUM_INPUT "%32s"
#define MAX 128
#define CONTACT_FILE "contacts.db"

typedef struct People {
    char name[NAME_LEN];
    char number[NUM_LEN];
} People;

People *People_new(const char *name, const char *num) {
    People *p = malloc(sizeof(People));
    strncpy(p->name, name, sizeof(p->name));
    strncpy(p->number, num, sizeof(p->number));
    return p;
}

void People_show(People *self) {
    if (self == NULL) {
        printf("NullPointerException\n");
        exit(-1);
    }

    printf("Name: %-8s, Number: %-11s\n",
        self->name,
        self->number);
}

//////////////////////////////////////////////////////////////

typedef struct {
    People *records[MAX];
} Vector;

Vector *Vec_new() {
    Vector *v = malloc(sizeof(Vector));
    memset(v, '\0', sizeof(Vector));
    if (!v) {
        printf("out of memory\n");
        exit(-1);
    }
    return v;
}

void Vec_add(Vector *self, People *product) {
    for (int i = 0; i < MAX; ++i) {
        if (self->records[i] == NULL) {
            self->records[i] = product;
            return;
        }
    }
    printf("Vec_add: out of space");
    exit(-1);
}

int Vec_indexOf(Vector *self, People *product) {
    for (int i = 0; i < MAX; ++i) {
        People *x = self->records[i];
        if (x == NULL) {
            continue;
        }
        if (product == x) {
            return i;
        }
    }
    return -1;
}

int Vec_indexOfByName(Vector *self, const char *name) {
    for (int i = 0; i < MAX; ++i) {
        People *product = self->records[i];
        if (product == NULL) {
            continue;
        }
        if (strncmp(name, product->name, sizeof(product->name)) == 0) {
            return i;
        }
    }
    return -1;
}

int Vec_indexOfByNumber(Vector *self, const char *number) {
    for (int i = 0; i < MAX; ++i) {
        People *product = self->records[i];
        if (product == NULL) {
            continue;
        }
        if (strncmp(number, product->number, sizeof(product->number)) == 0) {
            return i;
        }
    }
    return -1;
}

People *Vec_get(Vector *self, int index) {
    if (index < 0 || index >= MAX) {
        return NULL;
    }
    return self->records[index];
}

People *Vec_getByName(Vector *self, const char *name) {
    return Vec_get(self, Vec_indexOfByName(self, name));
}

void Vec_removeAt(Vector *self, int index) {
    if (index < 0 || index >= MAX) {
        return;
    }
    self->records[index] = NULL;
}

void Vec_remove(Vector *self, People *product) {
    Vec_removeAt(self, Vec_indexOf(self, product));
}

void Vec_show(Vector *self) {
    for (int i = 0; i < MAX; ++i) {
        People *product = self->records[i];
        if (product == NULL) {
            continue;
        }
        People_show(product);
    }
}

void Vec_show_with_index(Vector *self, int start) {
    for (int i = 0; i < MAX; ++i) {
        People *product = self->records[i];
        if (product == NULL) {
            continue;
        }
        printf(" %3d - ", i + start);
        People_show(product);
    }
}

void Vec_serialize_stream(Vector *self, FILE *out) {
    for (int i = 0; i < MAX; ++i) {
        People *product = self->records[i];
        if (product == NULL) {
            continue;
        }
        fwrite(product, sizeof(People), 1, out);
    }
}

void Vec_serialize_file(Vector *self, const char *file) {
    FILE *fp = fopen(file, "wb");
    if (!fp) {
        printf("open %s failed\n", file);
        exit(-1);
    }
    Vec_serialize_stream(self, fp);
    fclose(fp);
}

void Vec_deserialize_stream(Vector *self, FILE *in) {
    int index = 0;
    while (!feof(in)) {
        People *product = malloc(sizeof(People));
        if (fread(product, sizeof(People), 1, in) == 1) {
            self->records[index++] = product;
        }
    }
}

void Vec_deserialize_file(Vector *self, const char *file) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        printf("open %s failed\n", file);
        exit(-1);
    }
    Vec_deserialize_stream(self, fp);
    fclose(fp);
}

////////////////////////////////////////////////////////////////////
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[1;1H");
#endif
}

int UI_read_int(const char *prompt) {
    int choice = -1;
    printf("%s: ", prompt);
    scanf("%d", &choice);
    getchar();
    return choice;
}

int UI_select(const char *title, const char *items[]) {
    clear_screen();
    printf("%s\n", title);
    for (int i = 0; i < strlen(title); ++i) {
        putchar('=');
    }
    putchar('\n');

    int select = 1;
    const char **ptr = items;
    while (*ptr) {
        printf("  %-2d - %s\n", select++, *ptr++);
    }

    printf("\n");
    return UI_read_int("Input your choice");
}

void UI_pause() {
    printf("Press Enter to continue\n");
    getchar();
}

void load(Vector *v) {
    FILE *fp = fopen(CONTACT_FILE, "rb");
    if (!fp) {
        return;
    }
    Vec_deserialize_stream(v, fp);
}

void save(Vector *v) {
    FILE *fp = fopen(CONTACT_FILE, "wb");
    if (!fp) {
        return;
    }
    Vec_serialize_stream(v, fp);
}

int command_show(Vector *v) {
    Vec_show_with_index(v, 1);
    printf("===================================\n\n");
    UI_pause();
    return COMMAND_OK;
}

int command_add(Vector *v) {
    People *people = People_new("<unnamed>", "<unspecified>");

    printf("Type name  : ");
    scanf(NAME_INPUT, people->name);
    getchar();
    printf("Type number: ");
    scanf(NUM_INPUT, people->number);
    getchar();

    Vec_add(v, people);

    printf("Added record (name: %s, number: %s)\n",
        people->name,
        people->number);
    UI_pause();
    return COMMAND_OK;
}

int command_search_name(Vector *v) {
    char name[NAME_LEN] = {0};
    printf("Type the name you want to search: ");
    scanf(NAME_INPUT, name);
    getchar();

    int index = Vec_indexOfByName(v, name);
    if (index == -1) {
        printf("No people named '%s'\n", name);
    } else {
        People *people = Vec_get(v, index);
        People_show(people);
    }

    UI_pause();
    return COMMAND_OK;
}

int command_search_number(Vector *v) {
    char number[NUM_LEN] = {0};
    printf("Type the number you want to search: ");
    scanf(NUM_INPUT, number);
    getchar();

    int index = Vec_indexOfByNumber(v, number);
    if (index == -1) {
        printf("No people has the number '%s'\n", number);
    } else {
        People *people = Vec_get(v, index);
        People_show(people);
    }

    UI_pause();
    return COMMAND_OK;
}

int command_change(Vector *v) {
    Vec_show_with_index(v, 1);
    printf("===================================\n\n");

    int choice = UI_read_int("Type the index that you want to change");
    People *people = Vec_get(v, choice - 1);
    if (people == NULL) {
        printf("Invalid index, please try again\n");
        UI_pause();
        return COMMAND_OK;
    }

    choice = UI_select("Select your option", (const char * []) {
        "Change Name",
        "Change Number",
        NULL,
    });

    char *ptr = NULL;
    --choice;
    switch (choice) {
        case 0:
            ptr = people->name;
            break;
        case 1:
            ptr = people->number;
            break;
        default:
            printf("Invalid choice, please try again\n");
            UI_pause();
            break;
    }

    printf("Type the new content: ");
    scanf("%s", ptr);
    getchar();

    printf("Changed record(name: %s, number: %s)\n",
        people->name,
        people->number);
    UI_pause();
    return COMMAND_OK;
}

int command_delete(Vector *v) {
    Vec_show_with_index(v, 1);
    printf("===================================\n\n");
    int choice = UI_read_int("Type the index that you want to delete");
    Vec_removeAt(v, choice - 1);
    return COMMAND_OK;
}

int command_exit(Vector *v) {
    printf("Goodbye\n");
    return COMMAND_ERR;
}

void main_ui(Vector *v) {
    typedef int(*command_fn)(Vector *);

    static const command_fn cmds[] = {
        command_show,
        command_add,
        command_search_name,
        command_search_number,
        command_change,
        command_delete,
        command_exit,
    };

    while (1) {
        int select = UI_select("Welcome to the Contact Manager",
            (const char *[]) {
                "Show all record(s)",
                "Add new record",
                "Search record by name",
                "Search record by number",
                "Change record",
                "Delete record",
                "Exit",
                NULL,
            });

        --select;
        if (select >= 0 && select < sizeof(cmds) / sizeof(command_fn)) {
            clear_screen();
            if (cmds[select](v) != 0) {
                break;
            }
        } else {
            printf("Invalid input, please try again\n");
            break;
        }
    }
}

int main() {
    Vector *v = Vec_new();
    load(v);
    main_ui(v);
    save(v);
    return 0;
}
