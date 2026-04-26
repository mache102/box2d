## impl 
Implement this to create b2ChainSegments:

```c
B2_API b2ShapeId b2CreateChainSegmentShape(b2BodyId bodyId, const b2ShapeDef* def, const b2Segment* segment);
```

A b2ChainSegment has:

```c
	/// The tail ghost vertex
	b2Vec2 ghost1;

	/// The line segment
	b2Segment segment;

	/// The head ghost vertex
	b2Vec2 ghost2;

	/// The owning chain shape index (internal usage only)
	int chainId;
```

Where the two ghost vertices do not need to be assigned initially, and chainId can be B2_NULL_INDEX or -1 as we are not creating chain shapes here. 


Also create

```c
B2_API void b2ChainSegment_SetGhostVertices(b2ShapeId shapeId, b2Vec2 ghost1, b2Vec2 ghost2);
B2_API void b2ChainSegment_SetGhostVertex1(b2ShapeId shapeId, b2Vec2 ghost1);
B2_API void b2ChainSegment_SetGhostVertex2(b2ShapeId shapeId, b2Vec2 ghost2);
```

Which lets us set the ghost vertices by referencing our b2ChainSegment shape via shapeId. 

ghost1 should be the vertex before the start of the line segment (ie start of the prev seg)
ghost2 should be the vertex after the end of the line segment (ie end of the next seg)

### impl = deletion


## benchmark

create a benchmark in sample_benchmark.cpp.

put together several b2ChainSegment shapes, setting ghost vertices appropriately. simulate a ball (size is fraction of each segment length) rolling over the seg. 
to ensure a closed loop, add two more consecutive segments: one starts from the last  point, going downwards. and another from the downwards point, going back to the first point.
