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

    // Render Market Depth Chart
    drawDepthChart(data.buys, data.sells);
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

    // Render Price Execution Chart
    drawPriceChart(trades);
  } catch (err) {
    console.error('Failed to load trade history:', err);
  }
}

// Draw Market Depth Chart on HTML5 Canvas
function drawDepthChart(buys, sells) {
  const canvas = document.getElementById('depthChart');
  if (!canvas || !canvas.parentElement) return;
  const ctx = canvas.getContext('2d');
  const dpr = window.devicePixelRatio || 1;
  const width = canvas.parentElement.clientWidth || 450;
  const height = canvas.parentElement.clientHeight || 250;

  canvas.width = width * dpr;
  canvas.height = height * dpr;
  if (ctx.resetTransform) ctx.resetTransform();
  else ctx.setTransform(1, 0, 0, 1, 0, 0);
  ctx.scale(dpr, dpr);

  ctx.clearRect(0, 0, width, height);

  // Background grid
  ctx.strokeStyle = '#1e293b';
  ctx.lineWidth = 1;
  for (let y = 30; y < height - 30; y += 45) {
    ctx.beginPath();
    ctx.moveTo(35, y);
    ctx.lineTo(width - 20, y);
    ctx.stroke();
  }

  // Aggregate by price level
  const buyLevels = {};
  (buys || []).forEach(b => {
    buyLevels[b.price] = (buyLevels[b.price] || 0) + b.quantity;
  });
  const sellLevels = {};
  (sells || []).forEach(s => {
    sellLevels[s.price] = (sellLevels[s.price] || 0) + s.quantity;
  });

  const sortedBuyPrices = Object.keys(buyLevels).map(Number).sort((a, b) => b - a); // high to low
  const sortedSellPrices = Object.keys(sellLevels).map(Number).sort((a, b) => a - b); // low to high

  if (sortedBuyPrices.length === 0 && sortedSellPrices.length === 0) {
    ctx.fillStyle = '#64748b';
    ctx.font = '13px Arial';
    ctx.textAlign = 'center';
    ctx.fillText('No active bids or asks in book to render depth', width / 2, height / 2);
    return;
  }

  // Cumulative depths
  const buyDepth = [];
  let cumBuy = 0;
  sortedBuyPrices.forEach(p => {
    cumBuy += buyLevels[p];
    buyDepth.push({ price: p, volume: cumBuy });
  });

  const sellDepth = [];
  let cumSell = 0;
  sortedSellPrices.forEach(p => {
    cumSell += sellLevels[p];
    sellDepth.push({ price: p, volume: cumSell });
  });

  const maxVolume = Math.max(cumBuy, cumSell, 1);
  const minPrice = sortedBuyPrices.length > 0 ? sortedBuyPrices[sortedBuyPrices.length - 1] * 0.98 : sortedSellPrices[0] * 0.95;
  const maxPrice = sortedSellPrices.length > 0 ? sortedSellPrices[sortedSellPrices.length - 1] * 1.02 : sortedBuyPrices[0] * 1.05;
  const priceRange = (maxPrice - minPrice) || 10;

  const padLeft = 45;
  const padRight = 20;
  const padTop = 20;
  const padBottom = 35;
  const plotWidth = width - padLeft - padRight;
  const plotHeight = height - padTop - padBottom;

  const getX = (price) => padLeft + ((price - minPrice) / priceRange) * plotWidth;
  const getY = (vol) => padTop + plotHeight - (vol / maxVolume) * plotHeight;

  // Draw Bids (Green Area)
  if (buyDepth.length > 0) {
    ctx.beginPath();
    const startX = getX(buyDepth[0].price);
    ctx.moveTo(startX, getY(0));

    buyDepth.forEach(pt => {
      const x = getX(pt.price);
      const y = getY(pt.volume);
      ctx.lineTo(x, y);
    });

    const endX = getX(buyDepth[buyDepth.length - 1].price);
    ctx.lineTo(endX, getY(0));
    ctx.closePath();

    const gradBuy = ctx.createLinearGradient(0, padTop, 0, height - padBottom);
    gradBuy.addColorStop(0, 'rgba(34, 197, 94, 0.45)');
    gradBuy.addColorStop(1, 'rgba(34, 197, 94, 0.05)');
    ctx.fillStyle = gradBuy;
    ctx.fill();

    ctx.strokeStyle = '#22c55e';
    ctx.lineWidth = 2;
    ctx.stroke();
  }

  // Draw Asks (Red Area)
  if (sellDepth.length > 0) {
    ctx.beginPath();
    const startX = getX(sellDepth[0].price);
    ctx.moveTo(startX, getY(0));

    sellDepth.forEach(pt => {
      const x = getX(pt.price);
      const y = getY(pt.volume);
      ctx.lineTo(x, y);
    });

    const endX = getX(sellDepth[sellDepth.length - 1].price);
    ctx.lineTo(endX, getY(0));
    ctx.closePath();

    const gradSell = ctx.createLinearGradient(0, padTop, 0, height - padBottom);
    gradSell.addColorStop(0, 'rgba(239, 68, 68, 0.45)');
    gradSell.addColorStop(1, 'rgba(239, 68, 68, 0.05)');
    ctx.fillStyle = gradSell;
    ctx.fill();

    ctx.strokeStyle = '#ef4444';
    ctx.lineWidth = 2;
    ctx.stroke();
  }

  // Axis Labels
  ctx.fillStyle = '#94a3b8';
  ctx.font = '10px Arial';
  ctx.textAlign = 'right';
  ctx.fillText(formatNumber(maxVolume), padLeft - 6, padTop + 10);
  ctx.fillText(formatNumber(Math.round(maxVolume / 2)), padLeft - 6, padTop + plotHeight / 2);
  ctx.fillText('0', padLeft - 6, height - padBottom);

  ctx.textAlign = 'center';
  ctx.fillText('₹' + minPrice.toFixed(1), padLeft, height - 12);
  ctx.fillText('₹' + ((minPrice + maxPrice) / 2).toFixed(1), padLeft + plotWidth / 2, height - 12);
  ctx.fillText('₹' + maxPrice.toFixed(1), width - padRight, height - 12);
}

// Draw Real-Time Trade Price Chart on HTML5 Canvas
function drawPriceChart(trades) {
  const canvas = document.getElementById('priceChart');
  if (!canvas || !canvas.parentElement) return;
  const ctx = canvas.getContext('2d');
  const dpr = window.devicePixelRatio || 1;
  const width = canvas.parentElement.clientWidth || 450;
  const height = canvas.parentElement.clientHeight || 250;

  canvas.width = width * dpr;
  canvas.height = height * dpr;
  if (ctx.resetTransform) ctx.resetTransform();
  else ctx.setTransform(1, 0, 0, 1, 0, 0);
  ctx.scale(dpr, dpr);

  ctx.clearRect(0, 0, width, height);

  // Background grid
  ctx.strokeStyle = '#1e293b';
  ctx.lineWidth = 1;
  for (let y = 30; y < height - 30; y += 45) {
    ctx.beginPath();
    ctx.moveTo(35, y);
    ctx.lineTo(width - 20, y);
    ctx.stroke();
  }

  if (!trades || trades.length === 0) {
    ctx.fillStyle = '#64748b';
    ctx.font = '13px Arial';
    ctx.textAlign = 'center';
    ctx.fillText('No executed trades recorded yet to display price action', width / 2, height / 2);
    return;
  }

  // Chronological order (oldest to newest)
  const sorted = [...trades].sort((a, b) => a.trade_id - b.trade_id);
  const prices = sorted.map(t => t.price);
  const minPrice = Math.min(...prices) * 0.98;
  const maxPrice = Math.max(...prices) * 1.02;
  const range = (maxPrice - minPrice) || 1;

  const padLeft = 45;
  const padRight = 20;
  const padTop = 20;
  const padBottom = 35;
  const plotWidth = width - padLeft - padRight;
  const plotHeight = height - padTop - padBottom;

  const getX = (idx) => padLeft + (sorted.length === 1 ? plotWidth / 2 : (idx / (sorted.length - 1)) * plotWidth);
  const getY = (price) => padTop + plotHeight - ((price - minPrice) / range) * plotHeight;

  // Draw area gradient
  ctx.beginPath();
  ctx.moveTo(getX(0), padTop + plotHeight);
  sorted.forEach((t, i) => {
    ctx.lineTo(getX(i), getY(t.price));
  });
  ctx.lineTo(getX(sorted.length - 1), padTop + plotHeight);
  ctx.closePath();

  const grad = ctx.createLinearGradient(0, padTop, 0, height - padBottom);
  grad.addColorStop(0, 'rgba(56, 189, 248, 0.35)');
  grad.addColorStop(1, 'rgba(56, 189, 248, 0.02)');
  ctx.fillStyle = grad;
  ctx.fill();

  // Draw price line
  ctx.beginPath();
  sorted.forEach((t, i) => {
    const x = getX(i);
    const y = getY(t.price);
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.strokeStyle = '#38bdf8';
  ctx.lineWidth = 2.5;
  ctx.stroke();

  // Draw trade points
  sorted.forEach((t, i) => {
    const x = getX(i);
    const y = getY(t.price);

    ctx.beginPath();
    ctx.arc(x, y, 4, 0, Math.PI * 2);
    ctx.fillStyle = '#4ade80';
    ctx.fill();
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 1.5;
    ctx.stroke();
  });

  // Price Axis Labels
  ctx.fillStyle = '#94a3b8';
  ctx.font = '10px Arial';
  ctx.textAlign = 'right';
  ctx.fillText('₹' + maxPrice.toFixed(2), padLeft - 6, padTop + 10);
  ctx.fillText('₹' + ((minPrice + maxPrice) / 2).toFixed(2), padLeft - 6, padTop + plotHeight / 2);
  ctx.fillText('₹' + minPrice.toFixed(2), padLeft - 6, height - padBottom);

  ctx.textAlign = 'center';
  ctx.fillText('#T' + sorted[0].trade_id, getX(0), height - 12);
  if (sorted.length > 1) {
    ctx.fillText('#T' + sorted[sorted.length - 1].trade_id, getX(sorted.length - 1), height - 12);
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
  refreshTradersList();
  if (currentViewedTrader) {
    loadTraderHistory(currentViewedTrader, false);
  }
}

let currentViewedTrader = '';

async function refreshTradersList() {
  try {
    const res = await fetch(`${API_BASE}/api/traders`);
    if (!res.ok) return;
    const traders = await res.json();
    const container = document.getElementById('traders-chips');
    if (!container) return;

    if (!traders || traders.length === 0) {
      container.innerHTML = '<span style="color:#64748b; font-size:12px;">(none yet)</span>';
      return;
    }

    container.innerHTML = traders.map(t => `
      <span class="badge active-badge" style="cursor:pointer; user-select:none;" onclick="selectTrader('${t}')">${t}</span>
    `).join('');
  } catch (err) {
    console.error('Failed to load traders list:', err);
  }
}

function selectTrader(name) {
  const input = document.getElementById('user-history-input');
  if (input) input.value = name;
  loadTraderHistory(name, true);
}

async function loadTraderHistory(traderName, showStatus = true) {
  if (!traderName) return;
  currentViewedTrader = traderName;

  try {
    const res = await fetch(`${API_BASE}/api/user-history?trader=${encodeURIComponent(traderName)}`);
    if (!res.ok) return;
    const data = await res.json();

    const statusElem = document.getElementById('user-history-status');
    const tbody = document.getElementById('user-history-body');

    if (showStatus && statusElem) {
      if (data.has_file) {
        statusElem.textContent = `Displaying history from file for: ${data.trader} (user_history/${data.trader}.txt)`;
        statusElem.style.color = '#4ade80';
      } else {
        statusElem.textContent = `No history file found for trader "${data.trader}". Please place an order first.`;
        statusElem.style.color = '#fbbf24';
      }
    }

    if (!data.history || data.history.length === 0) {
      tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; color:#64748b;">No history records found for ${data.trader}</td></tr>`;
      return;
    }

    tbody.innerHTML = data.history.map(h => {
      let badgeStyle = 'background:#1e3a5f; color:#38bdf8;';
      if (h.action === 'PLACED') badgeStyle = 'background:#123326; color:#4ade80;';
      else if (h.action === 'EXECUTED') badgeStyle = 'background:#2e1065; color:#c084fc;';
      else if (h.action === 'CANCELLED') badgeStyle = 'background:#451a1a; color:#fca5a5;';
      else if (h.action === 'MODIFIED') badgeStyle = 'background:#3b2d16; color:#fbbf24;';

      const sideClass = h.side === 'BUY' ? 'green-text' : (h.side === 'SELL' ? 'red-text' : '');

      return `
        <tr>
          <td style="white-space:nowrap; font-size:12px;">${h.timestamp}</td>
          <td><span class="badge" style="${badgeStyle}">${h.action}</span></td>
          <td>#${h.order_id}</td>
          <td class="${sideClass}">${h.side}</td>
          <td>${h.price > 0 ? formatCurrency(h.price) : '-'}</td>
          <td>${h.quantity > 0 ? formatNumber(h.quantity) : '-'}</td>
          <td>${h.counterparty || '-'}</td>
          <td style="font-size:12px; color:#cbd5e1;">${h.details}</td>
        </tr>
      `;
    }).join('');
  } catch (err) {
    console.error('Failed to load trader history:', err);
  }
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

  // User History Load Button
  const loadHistoryBtn = document.getElementById('load-history-btn');
  const userHistoryInput = document.getElementById('user-history-input');
  if (loadHistoryBtn && userHistoryInput) {
    loadHistoryBtn.addEventListener('click', () => {
      const trader = userHistoryInput.value.trim();
      if (trader) {
        loadTraderHistory(trader, true);
      }
    });

    userHistoryInput.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') {
        e.preventDefault();
        const trader = userHistoryInput.value.trim();
        if (trader) {
          loadTraderHistory(trader, true);
        }
      }
    });
  }

  // Window resize re-draws charts cleanly
  window.addEventListener('resize', () => {
    refreshOrderBook();
    refreshTrades();
  });

  // Initial load
  refreshAll();

  // Periodic polling every 1.5 seconds for live real-time updates
  setInterval(refreshAll, 1500);
});

