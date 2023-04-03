//I made a few git mistakes and messed up my reposity, this was the code I wrote while i was working on the original repository. I had to force a git reset and force the push to the repository which got rid of my commit history but I made sure to backup my code which you see here. 

#include <stdlib.h>
#include "linked_list.h"

// Creates and returns a new list
// If compare is NULL, list_insert just inserts at the head
list_t* list_create(compare_fn compare)
{
    list_t* listp = (list_t*)malloc(sizeof(list_t)); 
    listp->head = NULL; 
    listp->tail = NULL; 
    listp->count = 0;
    listp->compare = compare;
    return listp;
}

// Destroys a list
void list_destroy(list_t* list)
{
    list_node_t* curr_node = list_head(list); 
    while(list_next(curr_node) != NULL)
    {
        list_node_t* node_to_remove = curr_node; 
        curr_node = curr_node->next; 
        list_remove(list, node_to_remove); 
    }
    list_node_t* node_to_remove = curr_node; 
    curr_node = NULL; 
    list_remove(list, node_to_remove); 

    list->head = NULL; 
    list->tail = NULL; 
    list->count = (size_t)NULL; 
    list->compare = NULL; 
    free(list); 
}

// Returns head of the list
list_node_t* list_head(list_t* list)
{
    if(list == NULL)
    {
        return NULL; 
    }
    return list->head;
}

// Returns tail of the list
list_node_t* list_tail(list_t* list)
{
    if(list == NULL)
    {
        return NULL; 
    }
    return list->tail;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    if(node == NULL)
    {
        return NULL; 
    }
    return node->next;
}

// Returns prev element in the list
list_node_t* list_prev(list_node_t* node)
{
    if(node == NULL)
    {
        return NULL; 
    }
    return node->prev;
}

// Returns end of the list marker
list_node_t* list_end(list_t* list)
{
    if(list == NULL)
    {
        return NULL; 
    }
    return list->tail;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    if(node == NULL)
    {
        return NULL; 
    }
    return node->data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    if(list == NULL)
    {
        return (size_t)NULL; 
    }
    return list->count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    if(list == NULL)
    {
        return NULL; 
    }

    list_node_t* curr_node = list_head(list); 
    int i = 0; 
    while(i<list_count(list))
    {
        if(curr_node->data == data)
        {
            return curr_node;
        }
        curr_node = curr_node->next;
        i++;
    }
    return NULL;
}

//Helper functions to insert nodes

int insert_new_node_before_head(list_t* list, list_node_t* new_node)
{
    if(list == NULL || new_node == NULL)
    {
        return NULL; 
    }
    new_node->next = list->head; 
    new_node->prev = NULL; 
    list->head->prev = new_node; 
    list->head = new_node; 
    list->count++; 
    return 1; 
}
int insert_new_node_after_tail(list_t* list, list_node_t* new_node)
{
    if(list == NULL || new_node == NULL)
    {
        return (int)NULL; 
    }
    new_node->next = NULL; 
    new_node->prev = list->tail; 
    list->tail->next = new_node; 
    list->tail = new_node; 
    list->count++; 
    return 1; 
}
int insert_new_node_after_node(list_t* list, list_node_t* new_node, list_node_t* list_node)
{
    if(list == NULL || new_node == NULL)
    {
        return (int)NULL; 
    }
    new_node->next = list_node->next; 
    new_node->prev = list_node; 
    list_node->next = new_node; 
    new_node->next->prev = new_node; 
    list->count++; 
    return 1; 
}


// Inserts a new node in the list with the given data
// Returns new node inserted
list_node_t* list_insert(list_t* list, void* data)
{
    if(list == NULL)
    {
        return (int)NULL; 
    }
    list_node_t* new_node = malloc(sizeof(list_node_t));
    new_node->data = data; 

    if(list->compare == NULL)
    {
        insert_new_node_before_head(list, new_node); 
        return new_node; 
    }


    
    return NULL;
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    list_node_t* curr_node = list_head(list); 
    while(curr_node != NULL)
    {
        if(curr_node == node)
        {
            if(list_count(list) == 1)
            {
                list->head = NULL; 
                list->tail = NULL; 
                free(curr_node); 
            }
            else if(list_count(list) == 2)
            {
                if(curr_node == list_head(list))
                {
                    list->head = list_tail(list);

                    list->tail->next = NULL;
                    list->tail->prev = NULL; 
                }
                else
                {
                    list->tail = list_head(list);

                    list->head->next = NULL; 
                    list->head->prev = NULL; 
                }
                free(curr_node); 
            }
            else if(curr_node == list_head(list))
            {
                curr_node->next->prev = NULL; 
                list->head = list_next(curr_node); 
                free(curr_node); 
            }
            else if(curr_node == list_tail(list))
            {
                curr_node->prev->next = NULL; 
                list->tail = list_prev(curr_node); 
                free(curr_node); 
            }
            else
            {
                list_node_t* prev_node = curr_node->prev;
                list_node_t* next_node = curr_node->next; 
                prev_node->next = next_node; 
                next_node->prev = prev_node; 
                free(curr_node); 
            }
            list->count--; 
        }
        curr_node = curr_node->next;
    }
}