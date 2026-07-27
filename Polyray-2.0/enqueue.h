// This is a personal academic project. Dear PVS-Studio, please check it.


/* enqueue.h

  Copyright (C) 2024, Clyde Meli, All rights reserved.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#if !defined(__POLYRAY_ENQUEUE)
#define __POLYRAY_ENQUEUE
#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <memory>
#include <stdexcept>
#include <gsl/gsl>
#include "defs3.h"
#include "memory.h"



//AngleBounds and CompositeObject were in bound.h originally
typedef struct t_angle_bounds AngleBounds;
struct t_angle_bounds {
    fVec U, V;         /* Basis vectors relating an object to a light */
    float umin, umax;  /* Angular bounds along the U axis */
    float vmin, vmax;  /* Angular bounds along the V axis */
};

/* This type overlays the standard object type - it is used to hold
   several primitive (or composite) objects in a single bounded
   container.  The type, bound, and parent components are the same
   size and offsets as in the Object data structure. */
#if 0
typedef struct t_composite_object {
    unsigned short   o_type;     //Holds the 'type' of the object 
    bbox_info        o_bnd;      //Bounding box limits 
    Object* o_parent;   //Parent object 
    unsigned short   c_size;     //Number of subobjects held 
    Object** c_object;   //Array of subobjects 
    //std::vector<Object*> c_object;//vector of subobjects. to use when all C has become C++
    AngleBounds* c_lbnd;     //Light/Eye angle bounds 
} CompositeObject;
#endif

#if 1
//todo: when shape sources are all C++
struct CompositeObject : public Object
{
    unsigned short   c_size;     //Number of subobjects held 
    //Object** c_object;   //Array of subobjects 
    //std::shared_ptr<Object>* c_object;
    //std::vector<std::unique_ptr<Object>> c_object;
    std::vector<Object *> c_object;
    AngleBounds* c_lbnd;     //Light/Eye angle bounds 
};
#endif

extern CompositeObject* FactoryCompositeObject(int size);

#ifdef TESTING
//extern void Set_Composite_Bounds(CompositeObject* cp);
#endif


struct QelemNew { //Priority Queue stucture
    Flt key;
    std::shared_ptr<Object> obj;
    std::shared_ptr<CompositeObject> cdp;

    bool operator<(const QelemNew& other) const {
        // We use greater-than to create a min-heap priority queue.
        return key > other.key;
    }
};



class PriorityQueue {
public:
    void enqueue(CompositeObject *cdp, std::shared_ptr<Object> obj, 
        float key) {
        //old form: std::shared_ptr<CompositeObject> share_cdp(new CompositeObject);
        auto share_cdp = std::make_shared<CompositeObject>();

        queue.emplace(QelemNew{ key, obj, share_cdp });
        if (queue.size() > maxQueueSize) {
            maxQueueSize = queue.size();
        }
        ++nEnqueued;
    }

    bool isEmpty() const {
        return queue.empty();
    }

    QelemNew dequeue() {
        if (queue.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        QelemNew elem = queue.top();
        queue.pop();
        return elem;
    }

private:
    std::priority_queue<QelemNew> queue;
    size_t maxQueueSize = 0;
    size_t nEnqueued = 0;
};


/*USAGE: int main() {
    PriorityQueue pq;
    auto cdp = std::make_shared<CompositeObject>();
    auto obj = std::make_shared<Object>();

    pq.enqueue(cdp, obj, 1.0f);
    pq.enqueue(cdp, obj, 0.5f);
    pq.enqueue(cdp, obj, 2.0f);

    while (!pq.isEmpty()) {
        Qelem elem = pq.dequeue();
        std::cout << "Dequeued element with key: " << elem.key << std::endl;
    }

    return 0;
}*/

#endif // __POLYRAY_ENQUEUE
