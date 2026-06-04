#include "chc_can_queue.h"

// --- Default Global Instance for Backward Compatibility ---
static S_CAN_HUB_INSTANCE _sDefaultHub;

// --- Private Helper Functions (Internal Logic) ---

static int _isQueueEmpty(const S_CAN_QUEUE_INSTANCE* pQueue) {
    return pQueue->head == pQueue->tail;
}

static int _isQueueFull(const S_CAN_QUEUE_INSTANCE* pQueue) {
    return ((pQueue->tail + 1) % QUEUE_SIZE) == pQueue->head;
}

static void _initQueue(S_CAN_QUEUE_INSTANCE* pQueue) {
    pQueue->head = 0;
    pQueue->tail = 0;
    pQueue->droppedCount = 0;
}

// --- Pointer-based API (Modern SDK) ---

void vCanQueue_initInstance(S_CAN_HUB_INSTANCE* pInst) {
    if (pInst == NULL) return;
    _initQueue(&pInst->sRxQueue);
    _initQueue(&pInst->sTxQueue);
}

int vCanQueue_enqueue(S_CAN_QUEUE_INSTANCE* pQueue, const CanFrame* pFrame) {
    if (pQueue == NULL || pFrame == NULL) return 0;
    
    int dropped = 0;
    if (_isQueueFull(pQueue)) {
        // Queue is full, drop the oldest message
        pQueue->head = (pQueue->head + 1) % QUEUE_SIZE;
        pQueue->droppedCount++;
        dropped = 1;
    }
    
    pQueue->buffer[pQueue->tail] = *pFrame;
    pQueue->tail = (pQueue->tail + 1) % QUEUE_SIZE;
    
    return dropped ? -1 : 1;
}

int vCanQueue_dequeue(S_CAN_QUEUE_INSTANCE* pQueue, CanFrame* pFrame) {
    if (pQueue == NULL || pFrame == NULL || _isQueueEmpty(pQueue)) {
        return 0;
    }
    
    *pFrame = pQueue->buffer[pQueue->head];
    pQueue->head = (pQueue->head + 1) % QUEUE_SIZE;
    
    return 1;
}

// --- Legacy API implementation (Maps to _sDefaultHub) ---

void initCanQueue(void) {
    vCanQueue_initInstance(&_sDefaultHub);
}

int enqueueRXQueue(const CanFrame *pFrame) {
    return vCanQueue_enqueue(&_sDefaultHub.sRxQueue, pFrame);
}

int dequeueRXQueue(CanFrame *pFrame) {
    return vCanQueue_dequeue(&_sDefaultHub.sRxQueue, pFrame);
}

int enqueueTXQueue(const CanFrame *pFrame) {
    return vCanQueue_enqueue(&_sDefaultHub.sTxQueue, pFrame);
}

int dequeueTXQueue(CanFrame *pFrame) {
    return vCanQueue_dequeue(&_sDefaultHub.sTxQueue, pFrame);
}

uint32_t getRXDroppedCount(void) {
    return _sDefaultHub.sRxQueue.droppedCount;
}

uint32_t getTXDroppedCount(void) {
    return _sDefaultHub.sTxQueue.droppedCount;
}
