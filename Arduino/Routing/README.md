### Arduino Routing 

Interface used to provide a routing layer for Arduino + XBee 802.15.4 Communication.

#### NONRouting

Provides a simple wrapper to the XbeeRadio. No routing is provided.

#### TREERouting

Two modes are availble for each device: `Node` and `Sink`.

`Sink` devices start the creation of a Tree Structure in the network using itself as the Root of the Tree.

`Node` devices start in a passive mode and wait for an invitation from either a `Sink` device or a `Node` connected to the sink.

`Node` devices continously search for the best (in terms of link quality) routes to the `Sink` (meaning that a `Node` will prefer a stornger local link instead of a more direct one).

Sending messagess is possibe from the `Sink` to any `Node` of the Tree and from any `Node` to the `Sink`.

No Message exchanges between `Node` devices is possible but it can be implemented externally. 
