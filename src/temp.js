var cn, x, lt; cn = 180; x = null; function u() { if (cn >= 0) { document.getElementById('t').innerHTML = 'Restart in ' + cn + ' seconds'; cn--; setTimeout(u, 1000); } } function c(l) { document.getElementById('s1').value = l.innerText || l.textContent; document.getElementById('p1').focus(); } function la(p) { var a = ''; if (la.arguments.length == 1) { a = p; clearTimeout(lt); } if (x != null) { x.abort(); } x = new XMLHttpRequest(); x.onreadystatechange = function () { if (x.readyState == 4 && x.status == 200) { var s = x.responseText.replace(/{t}/g, "<table style='width:100%'>").replace(/{s}/g, "<tr><th>").replace(/{m}/g, "</th><td>").replace(/{e}/g, "</td></tr>").replace(/{c}/g, "%'><div style='text-align:center;font-weight:"); document.getElementById(' l1').innerHTML = s; } }; x.open('GET', 'ay' + a, true); x.send(); lt = setTimeout(la, 2345); } function lb(p) { la('?d=' + p); } function lc(p) { la('?t=' + p); }
