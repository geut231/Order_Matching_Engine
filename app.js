// Base API URL (relative so works on any host/port)
const API_BASE = '';

// Format currency
function formatCurrency(val) {
  return '₹' + Number(val || 0).toLocaleString('en-IN', {
    minimumFractionDigits: 2,
    maximumFractionDigits: 2
  });
}

// Format numbers
function formatNumber(val) {
  return Number(val || 0).toLocaleString('en-IN');
}

// Refresh Market Summary & Stats
async function refreshMarketSummary() {
  try {
    const res = await fetch(`${API_BASE}/api/market-summary`);
    if (!res.ok) return;
    const data = await res.json();

    // Stats
    document.getElementById('stat-active-orders').textContent = formatNumber(data.active_orders);
    document.getElementById('stat-executed-trades').textContent = formatNumber(data.executed_trades);
    document.getElementById('stat-traded-qty').textContent = formatNumber(data.traded_quantity);
    document.getElementById('stat-traded-value').textContent = formatCurrency(data.traded_value);

    // Market Summary
    document.getElementById('market-best-bid').textContent = data.best_bid > 0 ? formatCurrency(data.best_bid) : '--';
    document.getElementById('market-best-ask').textContent = data.best_ask > 0 ? formatCurrency(data.best_ask) : '--';
    document.getElementById('market-spread').textContent = (data.best_bid > 0 && data.best_ask > 0) ? formatCurrency(data.spread) : '--';
    document.getElementById('market-mid-price').textContent = (data.best_bid > 0 || data.best_ask > 0) ? formatCurrency(data.mid_price) : '--';
  } catch (err) {
    console.error('Failed to load market summary:', err);
  }
}

// Refresh Order Book (Buys & Sells)
async function refreshOrderBook() {
  try {
    const res = await fetch(`${API_BASE}/api/order-book`);
    if (!res.ok) return;
    const data = await res.json();

    // Render Buys
    const buysTbody = document.getElementById('order-book-buys');
    if (!data.buys || data.buys.length === 0) {
      buysTbody.innerHTML = '<tr><td colspan="5" style="text-align:center; color:#64748b;">No buy orders</td></tr>';
    } else {
      buysTbody.innerHTML = data.buys.map(o => `
        <tr>
          <td>#${o.id}</td>
          <td class="green-text">${formatCurrency(o.price)}</td>
          <td>${formatNumber(o.quantity)}</td>
          <td>${o.order_no}</td>
          <td>${o.trader}</td>
        </tr>
      `).join('');
    }

    // Render Sells
    const sellsTbody = document.getElementById('order-book-sells');
    if (!data.sells || data.sells.length === 0) {
      sellsTbody.innerHTML = '<tr><td colspan="5" style="text-align:center; color:#64748b;">No sell orders</td></tr>';
    } else {
      sellsTbody.innerHTML = data.sells.map(o => `
        <tr>
          <td>#${o.id}</td>
          <td class="red-text">${formatCurrency(o.price)}</td>
          <td>${formatNumber(o.quantity)}</td>
          <td>${o.order_no}</td>
          <td>${o.trader}</td>
        </tr>
      `).join('');
    }
  } catch (err) {
    console.error('Failed to load order book:', err);
  }
}

// Refresh Active Orders
async function refreshActiveOrders() {
  try {
    const res = await fetch(`${API_BASE}/api/active-orders`);
    if (!res.ok) return;
    const orders = await res.json();

    const tbody = document.getElementById('active-orders-body');
    if (!orders || orders.length === 0) {
      tbody.innerHTML = '<tr><td colspan="7" style="text-align:center; color:#64748b;">No active orders</td></tr>';
      return;
    }

    tbody.innerHTML = orders.map(o => {
      const isBuy = o.side === 'BUY';
      const isPartial = o.status.includes('PARTIAL');
      const badgeClass = isPartial ? 'partial-badge' : 'active-badge';
      return `
        <tr>
          <td>#${o.id}</td>
          <td>${o.trader}</td>
          <td class="${isBuy ? 'green-text' : 'red-text'}">${o.side}</td>
          <td>${formatCurrency(o.price)}</td>
          <td>${formatNumber(o.quantity)} / ${formatNumber(o.original_qty)}</td>
          <td><span class="badge ${badgeClass}">${o.status}</span></td>
          <td>
            <button class="cancel-btn" onclick="cancelOrder(${o.id})">Cancel</button>
          </td>
        </tr>
      `;
    }).join('');
  } catch (err) {
    console.error('Failed to load active orders:', err);
  }
}

// Refresh Trade History
async function refreshTrades() {
  try {
    const res = await fetch(`${API_BASE}/api/trades`);
    if (!res.ok) return;
    const trades = await res.json();

    const tbody = document.getElementById('trade-history-body');
    if (!trades || trades.length === 0) {
      tbody.innerHTML = '<tr><td colspan="7" style="text-align:center; color:#64748b;">No trades recorded yet</td></tr>';
      return;
    }

    tbody.innerHTML = trades.map(t => `
      <tr>
        <td>#T${t.trade_id}</td>
        <td>#${t.buy_order_id}</td>
        <td>#${t.sell_order_id}</td>
        <td>${formatCurrency(t.price)}</td>
        <td>${formatNumber(t.quantity)}</td>
        <td>${formatCurrency(t.value)}</td>
        <td>${t.time || '-'}</td>
      </tr>
    `).join('');
  } catch (err) {
    console.error('Failed to load trade history:', err);
  }
}

// Cancel an Order
async function cancelOrder(id) {
  if (!confirm(`Are you sure you want to cancel order #${id}?`)) return;

  try {
    const res = await fetch(`${API_BASE}/api/cancel`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ id: Number(id) })
    });
    const result = await res.json();
    alert(result.message);
    refreshAll();
  } catch (err) {
    alert('Error cancelling order: ' + err.message);
  }
}

// Refresh all sections
function refreshAll() {
  refreshMarketSummary();
  refreshOrderBook();
  refreshActiveOrders();
  refreshTrades();
}

// Setup Event Listeners
document.addEventListener('DOMContentLoaded', () => {
  // Buy Form Submit
  const buyForm = document.getElementById('buy-order-form');
  buyForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const trader = document.getElementById('buy-trader').value.trim();
    const price = parseFloat(document.getElementById('buy-price').value);
    const quantity = parseInt(document.getElementById('buy-qty').value, 10);

    try {
      const res = await fetch(`${API_BASE}/api/orders`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ side: 'BUY', trader, price, quantity })
      });
      const data = await res.json();
      if (data.success) {
        buyForm.reset();
        refreshAll();
      } else {
        alert(data.message || 'Failed to place buy order.');
      }
    } catch (err) {
      alert('Network error placing buy order: ' + err.message);
    }
  });

  // Sell Form Submit
  const sellForm = document.getElementById('sell-order-form');
  sellForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const trader = document.getElementById('sell-trader').value.trim();
    const price = parseFloat(document.getElementById('sell-price').value);
    const quantity = parseInt(document.getElementById('sell-qty').value, 10);

    try {
      const res = await fetch(`${API_BASE}/api/orders`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ side: 'SELL', trader, price, quantity })
      });
      const data = await res.json();
      if (data.success) {
        sellForm.reset();
        refreshAll();
      } else {
        alert(data.message || 'Failed to place sell order.');
      }
    } catch (err) {
      alert('Network error placing sell order: ' + err.message);
    }
  });

  // Initial load
  refreshAll();

  // Periodic polling every 1.5 seconds for live real-time updates
  setInterval(refreshAll, 1500);
});

