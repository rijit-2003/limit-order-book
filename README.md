# Limit Order Book & Matching Engine (C++)

A compact, high-performance simulation of an electronic exchange’s matching engine.  
Supports **limit orders**, **market orders**, **partial fills**, **price–time priority**,  
**O(1) cancellation**, and **VWAP computation** — similar to the core of real-world trading systems  
used in exchanges and high-frequency trading (HFT) environments.

---

## 🚀 Features

### ✔️ Limit Orders
Buy/sell orders with a specified price are added to the book and respect:
- best price priority  
- FIFO (time priority) within a price level  

### ✔️ Market Orders
Market orders execute immediately against the best available prices.  
Unfilled quantities are discarded (standard exchange behavior).

### ✔️ Partial & Full Fills
Orders are matched until either the incoming or resting order is fully filled.

### ✔️ Price–Time Priority
- Bids sorted with **highest price first**
- Asks sorted with **lowest price first**
- FIFO queue within each price level

### ✔️ O(1) Order Cancellation (Optimized)
Each resting order stores an iterator into its price level, so cancellation is:
