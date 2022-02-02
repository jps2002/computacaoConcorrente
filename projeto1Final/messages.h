
/**
 * @brief Represents a node from queue of messages.
 * 
 */
typedef struct messageAux
{
    struct messageAux* next_node; // the next message node in the queue
    char* content; // the string containing the message itself
    int sender_id; // the identifier of the sender (optional)
    int reciever_id; // the identifier of the reciever (optional)
    int message_id; // the identifier of the message (optional)
} message;

/**
 * @brief Represents a queue of messages.
 * 
 */
typedef struct messageQueueAux
{
    message* head_node; // pointer to first message node heading the queue
    message* tail_node; // pointer to last message node in the queue
} messageQueue;



// function prototypes
void printMessageQueue(messageQueue* queue);
messageQueue* createQueue();
void enqueueMessage(char* content, int sender_id, int reciever_id, messageQueue* queue);
char* dequeueMessage(messageQueue* queue);
char* frontMessage(messageQueue* queue);

