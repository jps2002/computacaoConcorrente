#include <stdio.h>
#include <stdlib.h>
#include "data.h"

#define KEY_NOT_FOUND_CODE -1

node* head_node = NULL;
node* tail_node = NULL;

/**
 * @brief Adds a new node cointaining client_socket to the end of the queue.
 * 
 * @param client_socket id to be added to the queue.
 */
void enqueue(int* client_socket)
{
    // create new node
    node* new_node = (node*) malloc(sizeof(node));
    new_node->client_socket = client_socket;
    new_node->next_node = NULL;

    // add note to end of queue
        // if queue is empty: the node created becomes the head of a queue
    if (!tail_node)
        head_node = new_node;
    else // if queue is not empty: prepare current tail to be replaced as tail by the created node
        tail_node->next_node = new_node;

    tail_node = new_node; 
}

/**
 * @brief Retrives content from head node and then remove it from the queue.
 * 
 * @return pointer to a client_socket field from the head node
 * @return NULL, if queue is empty
 */
int* dequeue()
{
    // if the list is empty: error and return
    if (!head_node) 
    {
        // printf("error: dequeue() -> can not remove node from empty queue\n");
        return NULL;
    }
    
    // if not:
        // retrieve client_socket field from head node to be returned
    int* result = head_node->client_socket; // pointer to client_socket in the head node

        // configure new head for the queue 
    node* temp = head_node;
    head_node = head_node->next_node;
    if (!head_node) 
        tail_node = NULL;

    free(temp); // free memory allocated for former head node

    return result; // return pointer to client_socket field from former head node
}

/**
 * @brief Create a linkedList object
 * 
 * @return linkedList* pointer to NULL, if failed; or pointer to new list, if successfully created
 */
linkedList* createList()
{
    // allocate memory for new list   
    linkedList* new_list;
    if (!(new_list= (linkedList*) malloc(sizeof(linkedList))))
    {
        printf("error: createList() --> malloc()\n");
        return NULL;
    }

    // fill in new list
    new_list->head = NULL;
    new_list->size = 0;

    return new_list;
}

/**
 * @brief Inserts toBeInserted in a new node at the end of list
 * 
 * @param toBeInserted 
 * @param list 
 * @return nodeFromList* pointer to the created node
 */
nodeFromList* insertList(int toBeInserted, linkedList* list)
{
    // allocate memory for new node
    nodeFromList* new_node;
    if (!( new_node = (nodeFromList*) malloc(sizeof(nodeFromList))))
    {
        printf("error: createList() --> malloc()\n");
        return NULL;
    }

    // fill in new node
    new_node->client_socket = toBeInserted;
    new_node->next_node = NULL; 
    
    // if list is empty, make new_node it´s head
    if (list->head == NULL)
    {
        list->head = new_node;
        return new_node;
    }

    // traverse to last node
    nodeFromList* current_node = list->head;
    
    while (current_node->next_node)
    {
        current_node = current_node->next_node;
    }

    // add new node to end of list
    current_node->next_node = new_node;
    printf(" hey2\n");
    // increment list size
    list->size++;

    return new_node;
}

int removeList(int toBeRemoved, linkedList* list)
{
    // search for node with toBeRemoved key
    nodeFromList* current_node = list->head;
    nodeFromList* anterior_node = NULL;
    while (current_node)
    {
        // if key was found: remove node containing it
        if (current_node->client_socket == toBeRemoved)
        {
            // if key was found on head node: setup new head node
            if (anterior_node == NULL)
            {
                list->head = current_node->next_node;
            } 
            else // else: setup anterior_node next_node to node after current
            {
                anterior_node->next_node = current_node->next_node;
            }

            // free memory allocated for node to be removed
            free(current_node);

            // decrement list size
            list->size--;

            return 1;
        }
        anterior_node = current_node;
        current_node = current_node->next_node;
    }

    // if key was not found: return not found code
    printf("removeList() --> key not found in list\n");
    return KEY_NOT_FOUND_CODE;
}








