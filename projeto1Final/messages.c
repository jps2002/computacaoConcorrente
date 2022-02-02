#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"

/**
 * @brief Create a message queue object
 * 
 * @return messageQueue* 
 */
messageQueue* createQueue()
{
    // allocate memory for new message queue
    messageQueue* new_queue = (messageQueue*) malloc(sizeof(messageQueue));
    if (!new_queue) 
    {
        printf("Error: initializeQueue() -> malloc()\n");
        return NULL;
    }
    // fill in new message queue 
    new_queue->head_node = NULL;
    new_queue->tail_node = NULL;
    
    return new_queue;
}

/**
 * @brief Prints content of queue.
 * 
 * @param queue the message queue to be printed
 */
void printMessageQueue(messageQueue* queue)
{
    message* current_node; // message node being printed at a time

    current_node = queue->head_node; // initiate analysing head node of queue
    printf("Message Queue:\n");
    while(current_node != NULL)
    {
        printf("Message: \"%s\"\nSender: %d\nReciever: %d\n", 
            current_node->content, current_node->sender_id, current_node->reciever_id);
        printf("---\n");
        current_node = current_node->next_node;
    }
}

/**
 * @brief Adds a message to the end of a message queue pointed by queue
 * 
 * @param content the string containing the message itself
 * @param sender_id the id of the message sender
 * @param reciever_id the id of the message reciever
 * @param queue queue to which created message will be added 
 */
void enqueueMessage(char* content, int sender_id, int reciever_id, messageQueue* queue)
{
    // create new node 
        // memory allocation for new message
    message* new_message = (message*) malloc(sizeof(message));
    if(!new_message) 
    {
        printf("Error: (message*) malloc(sizeof(message)\n");
        return;
    }
        // fill in new message with passed information
    new_message->content = content;
    new_message->reciever_id = reciever_id;
    new_message->sender_id = sender_id;

    // add node to end of queue
    if (!queue->tail_node)
        queue->head_node = new_message;
    else // if queue is not empty: prepare current tail to be replaced as tail by the created node
        queue->tail_node->next_node = new_message;

    queue->tail_node = new_message; 
}

/**
 * @brief Retrives content from head node of queue and then remove it from queue.
 * 
 * @param queue the message queue from which message node will be removed
 * @return char* the string of former head node
 */
char* dequeueMessage(messageQueue* queue)
{
    printf("HEYUEYDWIOWEIFEOIF\n");
    // if the list is empty: error and return
    if (!queue->head_node) 
    {
        // printf("error: dequeue() -> can not remove node from empty queue\n");
        return NULL;
    }
    
    // if not:
        // retrieve content field from head node to be returned
    char* result = queue->head_node->content; // pointer to content of message in the head node
    printf("------->result %s\n", result);
        // configure new head for the queue 
    message* temp = queue->head_node;
    queue->head_node = queue->head_node->next_node;
    if (!queue->head_node) 
        queue->tail_node = NULL;

    free(temp); // free memory allocated for former head node

    return result; // return pointer to content field from former head node
    
}

/**
 * @brief Retrives content from head node of queue and whithout removing it from queue.
 * 
 * @param queue the message queue from which message node will be retrived
 * @return char* the string in head node
 */
char* frontMessage(messageQueue* queue)
{
    // if the list is empty: error and return
    if (!queue->head_node) 
    {
        // printf("error: front() -> can not remove node from empty queue\n");
        return NULL;
    }
    char* result = queue->head_node->content; // pointer to content of message in the head node
    return result; 
}
