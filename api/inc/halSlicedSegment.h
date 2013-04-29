/*
 * Copyright (C) 2013 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef _HALSLICEDSEGMENT_H
#define _HALSLICEDSEGMENT_H

#include "halDefs.h"
#include "halSegment.h"

namespace hal {

/** 
 * Interface for a sliced segement.  This extends the segment interface
 * by allowing slicing (accessing just subintervals of the segmenet), 
 * along with reversing. 
 */
class SlicedSegment : public virtual Segment
{
public:

   /** switch to segment's reverse complement */
   virtual void toReverse() const = 0;

   /** Get the iterator's start offset.  This is used when moving
    * vertically and following the parse index.  Any part of the
    * segment before the start offset is ignored by the iterator */  
   virtual hal_offset_t getStartOffset() const = 0;

   /** Get the iterator's end offset.  This is used when moving
    * vertically and following the parse index.  Any part of the
    * segment after the end offset is ignored by the iterator */  
   virtual hal_offset_t getEndOffset() const = 0;

   /** Set the iterator's start and end offsets
    * @param startOffset offset from beginning of segment
    * @param endOffset offset from end of segment */
   virtual void slice(hal_offset_t startOffset = 0,
                      hal_offset_t endOffset = 0) const = 0;


   /** Check whether iterator is on segment's reverse complement */
   virtual bool getReversed() const = 0;


protected:
   friend class counted_ptr<SlicedSegment>;
   friend class counted_ptr<const SlicedSegment>;
   virtual ~SlicedSegment() = 0;
};

inline SlicedSegment::~SlicedSegment() {}

inline bool operator<(SlicedSegmentConstPtr segmentIt,
                      hal_index_t genomePos) 
{
  return segmentIt->leftOf(genomePos);
}

inline bool operator>(SlicedSegmentConstPtr segmentIt,
                      hal_index_t genomePos) 
{
  return segmentIt->rightOf(genomePos);
}

}
#endif
