#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define QUEUE_SIZE 128  // Define the size of the queue (憓憭批隞交撠辣)

// Structure representing a CAN frame
typedef struct {
    uint32_t id;      // CAN Identifier
    uint8_t  dlc;        // Data Length Code
    uint8_t  data[8]; // Data Payload
} CanFrame;

// --- Instance-based Queue Definitions ---

// Structure representing a single queue instance
typedef struct {
    CanFrame buffer[QUEUE_SIZE];  // Buffer to store CAN frames
    int head;                     // Index of the head of the queue
    int tail;                     // Index of the tail of the queue
    volatile uint32_t droppedCount; // Counter for dropped messages
} S_CAN_QUEUE_INSTANCE;

// Structure representing a full CAN Hub (RX + TX)
typedef struct {
    S_CAN_QUEUE_INSTANCE sRxQueue;
    S_CAN_QUEUE_INSTANCE sTxQueue;
} S_CAN_HUB_INSTANCE;

// --- Pointer-based API (Modern SDK Style) ---

/**
 * @brief Initialize a CAN Hub instance
 */
void vCanQueue_initInstance(S_CAN_HUB_INSTANCE* pInst);

/**
 * @brief Enqueue a frame into a specific queue instance
 * @return 1 if successful, -1 if queue was full (oldest message dropped)
 */
int vCanQueue_enqueue(S_CAN_QUEUE_INSTANCE* pQueue, const CanFrame* pFrame);

/**
 * @brief Dequeue a frame from a specific queue instance
 * @return 1 if successful, 0 if queue is empty
 */
int vCanQueue_dequeue(S_CAN_QUEUE_INSTANCE* pQueue, CanFrame* pFrame);

// --- Legacy API (Backward Compatibility) ---
// These will map to a default global instance
void initCanQueue(void);

// Enqueue frame into RX queue
// Returns: 1 if successful, -1 if queue was full (oldest message dropped)
int enqueueRXQueue(const CanFrame *frame);

// Dequeue frame from RX queue
// Returns: 1 if successful, 0 if queue is empty
int dequeueRXQueue(CanFrame *frame);

// Enqueue frame into TX queue
// Returns: 1 if successful, -1 if queue was full (oldest message dropped)
int enqueueTXQueue(const CanFrame *frame);

// Dequeue frame from TX queue
// Returns: 1 if successful, 0 if queue is empty
int dequeueTXQueue(CanFrame *frame);

// Get statistics
uint32_t getRXDroppedCount(void);
uint32_t getTXDroppedCount(void);

#ifdef __cplusplus
}
#endif

#endif // CAN_QUEUE_H
