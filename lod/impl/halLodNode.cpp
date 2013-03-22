/*
 * Copyright (C) 2013 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */
#include <cmath>
#include <cassert>
#include "halLodNode.h"
#include "halLodEdge.h"

using namespace std;
using namespace hal;

LodNode::LodNode() : _sequence(NULL), _startPosition(NULL_INDEX), 
                     _endPosition(NULL_INDEX)
{

}

LodNode::LodNode(const Sequence* sequence, hal_index_t start,
                 hal_index_t last) : _sequence(sequence), 
                                     _startPosition(start), 
                                     _endPosition(last)
{
  assert(_sequence != NULL);
  assert(_startPosition <= _endPosition);
  assert(_startPosition >= _sequence->getStartPosition());
  assert(_startPosition <= _sequence->getEndPosition());
  assert(_endPosition >= _sequence->getStartPosition());
  assert(_endPosition <= _sequence->getEndPosition());
}

LodNode::~LodNode()
{
  set<LodEdge*> tempSet;
  

  for (EdgeIterator i = _edges.begin(); i != _edges.end(); ++i)
  {    
    assert(tempSet.find(*i) == tempSet.end());
    tempSet.insert(*i);
    
    LodEdge* edge = *i;
    LodNode* other = edge->getOtherNode(this);
    if (other == NULL || other == this)
    {
      delete edge;
    }
    else
    {
      edge->nullifyNode(this);
    }
  }
}

void LodNode::addEdge(const Sequence* sequence,
                      hal_index_t srcPos, bool srcReversed, 
                      LodNode* tgt, hal_index_t tgtPos, bool tgtReversed)
{
  LodEdge* edge = new LodEdge(sequence, 
                              this, srcPos, srcReversed,
                              tgt, tgtPos, tgtReversed);
  _edges.push_back(edge);
  if (tgt != NULL && tgt != this)
  {
    tgt->_edges.push_back(edge);
  }
}

void LodNode::extend(double extendFraction)
{
  hal_size_t fMin;
  hal_size_t rMin; 
  getMinLengths(fMin, rMin);
  hal_size_t fLen = (hal_size_t)std::ceil((double)fMin * extendFraction);
  hal_size_t rLen = (hal_size_t)std::ceil((double)rMin * extendFraction);

  if (fLen > 0 || rLen > 0)
  {
    bool thisRev;
    LodNode* other;
    LodEdge* edge;
    for (EdgeIterator i = _edges.begin(); i != _edges.end(); ++i)
    {    
      edge = *i;
      other = edge->getOtherNode(this, &thisRev);
      bool left = edge->getStartPosition(this) < edge->getEndPosition(this);
      hal_size_t delta = thisRev ? rLen : fLen;

      edge->shrink(delta, left);
    }
    
    assert(_startPosition >= (hal_index_t)rLen);
    _startPosition -= (hal_index_t)rLen;
    
    assert(_endPosition + fLen <= (hal_size_t)_sequence->getEndPosition());
    _endPosition += (hal_index_t)fLen;
  }
}

void LodNode::fillInEdges(vector<LodNode*>& nodeBuffer)
{
  vector<LodEdge*> fBatch;
  vector<LodEdge*> rBatch;
  const Sequence* sequence = NULL;
  hal_index_t start = NULL_INDEX;
  EdgeIterator cur = _edges.begin();
  bool rev;
  while (true)
  {
    if (cur == _edges.end() || 
        (sequence != NULL && (*cur)->_sequence != sequence) ||
        (start != NULL_INDEX && (*cur)->getStartPosition(this) != start))
    {
      if (!fBatch.empty())
      {
        insertFillNode(fBatch, nodeBuffer);
      }
      if (!rBatch.empty())
      {
        insertFillNode(rBatch, nodeBuffer);
      }
      fBatch.clear();
      rBatch.clear();
    }
    if (cur == _edges.end())
    {
      break;
    }
    else
    {
      sequence = (*cur)->getSequence();
      start = (*cur)->getStartPosition(this);
      (*cur)->getOtherNode(this, &rev, NULL);
      if (rev)
      {
/*        if (!rBatch.empty())
        {
          cout << **cur << endl
               << (*cur)->getNode1()->getSequence()->getName() << "->"
               << (*cur)->getNode2()->getSequence()->getName() << "\n"
               << rBatch.back() << endl 
               << rBatch.back()->getSequence()->getName() << "->"
               << rBatch.back()->getSequence()->getName() << "\n"
               << endl;
               }*/
        assert(rBatch.empty() || 
               (*cur)->getLength() == rBatch.back()->getLength());
        rBatch.push_back(*cur);
      }
      else
      {
/*
        if (!fBatch.empty())
        {
          cout << **cur << endl
               << (*cur)->getNode1()->getSequence()->getName() 
               << " l=" << (*cur)->getNode1()->getLength() << "->"
               << (*cur)->getNode2()->getSequence()->getName()
               << " l=" << (*cur)->getNode2()->getLength() << "\n"
               << *fBatch.back() << endl 
               << fBatch.back()->getSequence()->getName() 
               << " l=" << fBatch.back()->getLength() << "->"
               << fBatch.back()->getSequence()->getName()  
               << " l=" << fBatch.back()->getLength() << "\n"
               << endl;
               }*/
        assert(fBatch.empty() || 
               (*cur)->getLength()== fBatch.back()->getLength());
        fBatch.push_back(*cur);
      }
    }
    ++cur;
  }
}

void LodNode::insertFillNode(vector<LodEdge*>& edgeBatch,
                             vector<LodNode*>& nodeBuffer)
{
  assert(edgeBatch.size() > 0);
  LodEdge* edge = edgeBatch.at(0);
  bool reverse;
  edge->getOtherNode(this, &reverse, NULL);
  hal_index_t newStart = edge->getStartPosition(this);
  
  hal_index_t newEnd = NULL_INDEX;
  
}

void LodNode::getEdgeLengthStats(hal_size_t& fMin, hal_size_t& fMax, 
                                 hal_size_t& fTot,
                                 hal_size_t& rMin, hal_size_t& rMax,
                                 hal_size_t& rTot) const
{
  fMin = numeric_limits<hal_size_t>::max();
  fMax = 0;
  fTot = 0;
  rMin = numeric_limits<hal_size_t>::max();
  rMax = 0;
  rTot = 0;  
  bool rev;

  for (EdgeConstIterator i = _edges.begin(); i != _edges.end(); ++i)
  {
    hal_size_t length = (*i)->getLength();
    (*i)->getOtherNode(this, &rev, NULL);
    if (rev == true)
    {
      rMin = min(length, rMin);
      rMax = max(length, rMax);
      rTot += length;
    }
    else
    {
      fMin = min(length, fMin);
      fMax = max(length, fMax);
      fTot += length;
    }
  }
}

void LodNode::getMinLengths(hal_size_t& fMin, hal_size_t& rMin) const
{
  fMin = numeric_limits<hal_size_t>::max();
  rMin = numeric_limits<hal_size_t>::max();  
  bool rev;

  for (EdgeConstIterator i = _edges.begin(); i != _edges.end(); ++i)
  {
    hal_size_t length = (*i)->getLength();
    (*i)->getOtherNode(this, &rev, NULL);
    if (rev == true)
    {
      rMin = min(length, rMin);
    }
    else
    {
      fMin = min(length, fMin);
    }
  }
}

ostream& hal::operator<<(ostream& os, const LodNode& node)
{
  os << "node " << &node << ": ";
  if (node.getSequence() != NULL)
  {
    os << node.getSequence()->getFullName();
  }
  else
  {
    os << "NULL";
  }
  os << "  (" << node.getStartPosition() << ", " << node.getEndPosition();
  os << ")";

  hal_size_t ecount = node.getDegree();
  if (ecount > 0)
  {
    os << "\n";
  }
  for (LodNode::EdgeList::const_iterator i = node._edges.begin(); 
       i != node._edges.end(); ++i)
  {
    os << "  " << ecount++ << ")" << **i << "\n";
  }
  return os;
}
