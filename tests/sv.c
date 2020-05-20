#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 32

typedef enum ProductKind {
    METAL,
    PLASTIC,
    MEAL,
    PLANT,
} ProductKind;

const char *ProductKind_show(ProductKind kind) {
    switch (kind) {
        case METAL:
            return "metal";
        case PLASTIC:
            return "plastic";
        case MEAL:
            return "meal";
        case PLANT:
            return "plant";
        default:
            printf("unreachable\n");
            exit(-1);
    }
}

typedef struct Product {
    char name[32];
    char unit[32];
    long time;
    ProductKind kind;
    double amount;
} Product;

Product *Product_new(long time, ProductKind kind,
                     const char *name, double amount,
                     const char *unit) {
    Product *p = malloc(sizeof(Product));
    p->time = time;
    p->kind = kind;
    p->amount = amount;
    strncpy(p->name, name, sizeof(p->name));
    strncpy(p->unit, unit, sizeof(p->unit));
    return p;
}

void Product_show(Product *this) {
    if (this == NULL) {
        printf("java.lang.NullPointerException\n");
        exit(-1);
    }

    printf("Time: %-8ld, Kind: %-8s, Name: %-8s, Amount: %.2lf (%s)\n",
        this->time, ProductKind_show(this->kind),
        this->name, this->amount, this->unit);
}

int product_time_cmp(const void *plhs, const void *prhs) {
    Product *lhs = *(Product **) plhs;
    Product *rhs = *(Product **) prhs;
    if (lhs == NULL) {
        return 1;
    }
    if (rhs == NULL) {
        return -1;
    }
    return (int) (lhs->time - rhs->time);
}

//////////////////////////////////////////////////////////////

typedef struct {
    Product *products[MAX];
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

void Vec_add(Vector *this, Product *product) {
    for (int i = 0; i < MAX; ++i) {
        if (this->products[i] == NULL) {
            this->products[i] = product;
            return;
        }
    }
    printf("Vec_add: out of space");
    exit(-1);
}

void Vec_sort(Vector *this) {
    qsort(this->products, MAX, sizeof(Product *), product_time_cmp);
}

int Vec_indexOf(Vector *this, Product *product) {
    for (int i = 0; i < MAX; ++i) {
        Product *x = this->products[i];
        if (x == NULL) {
            continue;
        }
        if (product == x) {
            return i;
        }
    }
    return -1;
}

int Vec_indexOfByName(Vector *this, const char *name) {
    for (int i = 0; i < MAX; ++i) {
        Product *product = this->products[i];
        if (product == NULL) {
            continue;
        }
        if (strncmp(name, product->name, sizeof(product->name)) == 0) {
            return i;
        }
    }
    return -1;
}

Product *Vec_get(Vector *this, int index) {
    if (index < 0 || index >= MAX) {
        return NULL;
    }
    return this->products[index];
}

Product *Vec_getByName(Vector *this, const char *name) {
    return Vec_get(this, Vec_indexOfByName(this, name));
}

void Vec_removeAt(Vector *this, int index) {
    if (index < 0 || index >= MAX) {
        return;
    }
    this->products[index] = NULL;
}

void Vec_remove(Vector *this, Product *product) {
    Vec_removeAt(this, Vec_indexOf(this, product));
}

void Vec_show(Vector *this) {
    for (int i = 0; i < MAX; ++i) {
        Product *product = this->products[i];
        if (product == NULL) {
            continue;
        }
        Product_show(product);
    }
}

void Vec_serialize_stream(Vector *this, FILE *out) {
    for (int i = 0; i < MAX; ++i) {
        Product *product = this->products[i];
        if (product == NULL) {
            continue;
        }
        fwrite(product, sizeof(Product), 1, out);
    }
}

void Vec_serialize_file(Vector *this, const char *file) {
    FILE *fp = fopen(file, "wb");
    if (!fp) {
        printf("open %s failed\n", file);
        exit(-1);
    }
    Vec_serialize_stream(this, fp);
    fclose(fp);
}

void Vec_deserialize_stream(Vector *this, FILE *in) {
    int index = 0;
    while (!feof(in)) {
        Product *product = malloc(sizeof(Product));
        if (fread(product, sizeof(Product), 1, in) == 1) {
            this->products[index++] = product;
        }
    }
}

void Vec_deserialize_file(Vector *this, const char *file) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        printf("open %s failed\n", file);
        exit(-1);
    }
    Vec_deserialize_stream(this, fp);
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
    printf("Input your choice: ");

    int choice = -1;
    scanf("%d", &choice);
    return choice;
}

void command_add(Vector *v) {

}

void command_search(Vector *v) {
}

void command_manage(Vector *v) {
}

void command_load(Vector *v) {
}

void command_store(Vector *v) {
}

void command_exit(Vector *v) {
    printf("Goodbye\n");
    exit(0);
}

void main_ui() {
    typedef void(*command_fn)(Vector *);

    static const command_fn cmds[] = {
        command_add,
        command_search,
        command_manage,
        command_load,
        command_store,
        command_exit,
    };

    Vector *v = Vec_new();

    while (1) {
        int select = UI_select("Welcome to the Commodity Outbound Management System",
            (const char *[]) {
                "Add new record(s)",
                "Search record(s)",
                "Manage record(s)",
                "Load from file",
                "Store to file",
                "Exit",
                NULL,
            });

        --select;
        if (select >= 0 && select < sizeof(cmds) / sizeof(command_fn)) {
            clear_screen();
            cmds[select](v);
        } else {
            printf("Invalid input, good bye\n");
            return;
        }
    }
}

int main() {
    main_ui();
}
