//
// Created by kiva on 2020/4/8.
//
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>

#define makeAny(v) ((Any) v)
#define foreach(list, x) for (Node *x = list->head; x; x = x->next)

typedef void *Any;

typedef void(*TraversalCallback)(Any);

typedef struct Node {
    Any element;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
} List;

typedef struct NodePair {
    Node *prev;
    Node *current;
} NodePair;

Node *newNode(Any value) {
    Node *node = malloc(sizeof(Node));
    node->element = value;
    node->next = NULL;
    return node;
}

void deleteNode(Node *node) {
    free(node);
}

List *newList() {
    List *list = malloc(sizeof(List));
    list->head = NULL;
    return list;
}

void deleteList(List *list) {
    foreach(list, cur) {
        free(cur);
    }
    free(list);
}

bool isEmpty(List *list) {
    return !list->head;
}

void insert(List *list, Any value) {
    Node *node = newNode(value);
    Node *tail = NULL;
    foreach(list, x) {
        tail = x;
    }
    if (tail == NULL) {
        list->head = node;
    } else {
        tail->next = node;
    }
}

NodePair internalFind(List *list, Any value) {
    Node *prev = NULL;
    Node *found = NULL;
    foreach(list, x) {
        if (x->element == value) {
            found = x;
            break;
        } else {
            prev = x;
        }
    }
    NodePair pair;
    pair.current = found;
    pair.prev = prev;
    return pair;
}

void delete(List *list, Any value) {
    if (isEmpty(list)) {
        return;
    }

    NodePair pair = internalFind(list, value);
    Node *found = pair.current;
    Node *prev = pair.prev;

    if (!found) {
        return;
    }

    if (!prev) {
        deleteNode(found);
        list->head = NULL;
    } else {
        prev->next = found->next;
        deleteNode(found);
    }
}

void delete2(List *list, Any value) {
    Node **indirect = &(list->head);
    while (*indirect && (*indirect)->element != value) {
        indirect = &((*indirect)->next);
    }

    if (!*indirect) {
        return;
    }

    Node *next = (*indirect)->next;
    deleteNode(*indirect);
    *indirect = next;
}

Node *find(List *list, Any value) {
    return internalFind(list, value).current;
}

int indexOf(List *list, Any value) {
    Node *node = find(list, value);
    if (!node) {
        return -1;
    }

    int position = 0;
    foreach(list, x) {
        if (x == node) {
            return position;
        }
        ++position;
    }
    return -1;
}

bool replace(List *list, Any old, Any new) {
    Node *node = find(list, old);
    if (node) {
        node->element = new;
        return true;
    }
    return false;
}

void replaceAll(List *list, Any old, Any new) {
    while (replace(list, old, new)) {
        // nothing
    }
}

void traverse(List *list, TraversalCallback fn) {
    foreach(list, x) {
        fn(x->element);
    }
}

void show(Any any) {
    printf("%d\n", (int) any);
}

int main() {
    List *list = newList();
    printf("\n1::\n");
    traverse(list, show);

    insert(list, (Any) 0);
    insert(list, (Any) 1);
    insert(list, (Any) 2);
    insert(list, (Any) 3);
    printf("\n2::\n");
    traverse(list, show);

    delete(list, (Any) 1);
    delete(list, (Any) 6);
    printf("\n3::\n");
    traverse(list, show);

    printf("\n4::\n");
    printf("indexOf 3: %d\n", indexOf(list, (Any) 3));
    printf("indexOf 0: %d\n", indexOf(list, (Any) 0));
    printf("indexOf 8: %d\n", indexOf(list, (Any) 8));

    printf("\n5::\n");
    insert(list, (Any) 9);
    insert(list, (Any) 9);
    insert(list, (Any) 8);
    insert(list, (Any) 7);
    insert(list, (Any) 9);
    insert(list, (Any) 8);
    traverse(list, show);

    printf("\n6::\n");
    replaceAll(list, (Any) 9, (Any) 1);
    replaceAll(list, (Any) 8, (Any) 4);
    replaceAll(list, (Any) 7, (Any) 5);
    traverse(list, show);

    printf("\n7::\n");
    delete2(list, (Any) 1);
    delete2(list, (Any) 1);
    delete2(list, (Any) 0);
    delete2(list, (Any) 1);
    traverse(list, show);

    deleteList(list);
}
