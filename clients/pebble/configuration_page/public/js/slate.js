/*! Sortable 1.2.0 - MIT | git://github.com/rubaxa/Sortable.git */
!function(a){"use strict";"function"==typeof define&&define.amd?define(a):"undefined"!=typeof module&&"undefined"!=typeof module.exports?module.exports=a():"undefined"!=typeof Package?Sortable=a():window.Sortable=a()}(function(){"use strict";function a(a,b){this.el=a,this.options=b=q({},b),a[H]=this;var d={group:Math.random(),sort:!0,disabled:!1,store:null,handle:null,scroll:!0,scrollSensitivity:30,scrollSpeed:10,draggable:/[uo]l/i.test(a.nodeName)?"li":">*",ghostClass:"sortable-ghost",ignore:"a, img",filter:null,animation:0,setData:function(a,b){a.setData("Text",b.textContent)},dropBubble:!1,dragoverBubble:!1,dataIdAttr:"data-id",delay:0};for(var e in d)!(e in b)&&(b[e]=d[e]);var g=b.group;g&&"object"==typeof g||(g=b.group={name:g}),["pull","put"].forEach(function(a){a in g||(g[a]=!0)}),b.groups=" "+g.name+(g.put.join?" "+g.put.join(" "):"")+" ";for(var h in this)"_"===h.charAt(0)&&(this[h]=c(this,this[h]));f(a,"mousedown",this._onTapStart),f(a,"touchstart",this._onTapStart),f(a,"dragover",this),f(a,"dragenter",this),Q.push(this._onDragOver),b.store&&this.sort(b.store.get(this))}function b(a){t&&t.state!==a&&(i(t,"display",a?"none":""),!a&&t.state&&u.insertBefore(t,r),t.state=a)}function c(a,b){var c=P.call(arguments,2);return b.bind?b.bind.apply(b,[a].concat(c)):function(){return b.apply(a,c.concat(P.call(arguments)))}}function d(a,b,c){if(a){c=c||J,b=b.split(".");var d=b.shift().toUpperCase(),e=new RegExp("\\s("+b.join("|")+")\\s","g");do if(">*"===d&&a.parentNode===c||(""===d||a.nodeName.toUpperCase()==d)&&(!b.length||((" "+a.className+" ").match(e)||[]).length==b.length))return a;while(a!==c&&(a=a.parentNode))}return null}function e(a){a.dataTransfer.dropEffect="move",a.preventDefault()}function f(a,b,c){a.addEventListener(b,c,!1)}function g(a,b,c){a.removeEventListener(b,c,!1)}function h(a,b,c){if(a)if(a.classList)a.classList[c?"add":"remove"](b);else{var d=(" "+a.className+" ").replace(G," ").replace(" "+b+" "," ");a.className=(d+(c?" "+b:"")).replace(G," ")}}function i(a,b,c){var d=a&&a.style;if(d){if(void 0===c)return J.defaultView&&J.defaultView.getComputedStyle?c=J.defaultView.getComputedStyle(a,""):a.currentStyle&&(c=a.currentStyle),void 0===b?c:c[b];b in d||(b="-webkit-"+b),d[b]=c+("string"==typeof c?"":"px")}}function j(a,b,c){if(a){var d=a.getElementsByTagName(b),e=0,f=d.length;if(c)for(;f>e;e++)c(d[e],e);return d}return[]}function k(a){a.draggable=!1}function l(){M=!1}function m(a,b){var c=a.lastElementChild,d=c.getBoundingClientRect();return b.clientY-(d.top+d.height)>5&&c}function n(a){for(var b=a.tagName+a.className+a.src+a.href+a.textContent,c=b.length,d=0;c--;)d+=b.charCodeAt(c);return d.toString(36)}function o(a){for(var b=0;a&&(a=a.previousElementSibling);)"TEMPLATE"!==a.nodeName.toUpperCase()&&b++;return b}function p(a,b){var c,d;return function(){void 0===c&&(c=arguments,d=this,setTimeout(function(){1===c.length?a.call(d,c[0]):a.apply(d,c),c=void 0},b))}}function q(a,b){if(a&&b)for(var c in b)b.hasOwnProperty(c)&&(a[c]=b[c]);return a}var r,s,t,u,v,w,x,y,z,A,B,C,D,E,F={},G=/\s+/g,H="Sortable"+(new Date).getTime(),I=window,J=I.document,K=I.parseInt,L=!!("draggable"in J.createElement("div")),M=!1,N=function(a,b,c,d,e,f,g){var h=J.createEvent("Event"),i=(a||b[H]).options,j="on"+c.charAt(0).toUpperCase()+c.substr(1);h.initEvent(c,!0,!0),h.item=d||b,h.from=e||b,h.clone=t,h.oldIndex=f,h.newIndex=g,i[j]&&i[j].call(a,h),b.dispatchEvent(h)},O=Math.abs,P=[].slice,Q=[],R=p(function(a,b,c){if(c&&b.scroll){var d,e,f,g,h=b.scrollSensitivity,i=b.scrollSpeed,j=a.clientX,k=a.clientY,l=window.innerWidth,m=window.innerHeight;if(x!==c&&(w=b.scroll,x=c,w===!0)){w=c;do if(w.offsetWidth<w.scrollWidth||w.offsetHeight<w.scrollHeight)break;while(w=w.parentNode)}w&&(d=w,e=w.getBoundingClientRect(),f=(O(e.right-j)<=h)-(O(e.left-j)<=h),g=(O(e.bottom-k)<=h)-(O(e.top-k)<=h)),f||g||(f=(h>=l-j)-(h>=j),g=(h>=m-k)-(h>=k),(f||g)&&(d=I)),(F.vx!==f||F.vy!==g||F.el!==d)&&(F.el=d,F.vx=f,F.vy=g,clearInterval(F.pid),d&&(F.pid=setInterval(function(){d===I?I.scrollTo(I.pageXOffset+f*i,I.pageYOffset+g*i):(g&&(d.scrollTop+=g*i),f&&(d.scrollLeft+=f*i))},24)))}},30);return a.prototype={constructor:a,_onTapStart:function(a){var b=this,c=this.el,e=this.options,f=a.type,g=a.touches&&a.touches[0],h=(g||a).target,i=h,j=e.filter;if(!("mousedown"===f&&0!==a.button||e.disabled)&&(h=d(h,e.draggable,c))){if(A=o(h),"function"==typeof j){if(j.call(this,a,h,this))return N(b,i,"filter",h,c,A),void a.preventDefault()}else if(j&&(j=j.split(",").some(function(a){return a=d(i,a.trim(),c),a?(N(b,a,"filter",h,c,A),!0):void 0})))return void a.preventDefault();(!e.handle||d(i,e.handle,c))&&this._prepareDragStart(a,g,h)}},_prepareDragStart:function(a,b,c){var d,e=this,g=e.el,h=e.options,i=g.ownerDocument;c&&!r&&c.parentNode===g&&(D=a,u=g,r=c,v=r.nextSibling,C=h.group,d=function(){e._disableDelayedDrag(),r.draggable=!0,h.ignore.split(",").forEach(function(a){j(r,a.trim(),k)}),e._triggerDragStart(b)},f(i,"mouseup",e._onDrop),f(i,"touchend",e._onDrop),f(i,"touchcancel",e._onDrop),h.delay?(f(i,"mousemove",e._disableDelayedDrag),f(i,"touchmove",e._disableDelayedDrag),e._dragStartTimer=setTimeout(d,h.delay)):d())},_disableDelayedDrag:function(){var a=this.el.ownerDocument;clearTimeout(this._dragStartTimer),g(a,"mousemove",this._disableDelayedDrag),g(a,"touchmove",this._disableDelayedDrag)},_triggerDragStart:function(a){a?(D={target:r,clientX:a.clientX,clientY:a.clientY},this._onDragStart(D,"touch")):L?(f(r,"dragend",this),f(u,"dragstart",this._onDragStart)):this._onDragStart(D,!0);try{J.selection?J.selection.empty():window.getSelection().removeAllRanges()}catch(b){}},_dragStarted:function(){u&&r&&(h(r,this.options.ghostClass,!0),a.active=this,N(this,u,"start",r,u,A))},_emulateDragOver:function(){if(E){i(s,"display","none");var a=J.elementFromPoint(E.clientX,E.clientY),b=a,c=" "+this.options.group.name,d=Q.length;if(b)do{if(b[H]&&b[H].options.groups.indexOf(c)>-1){for(;d--;)Q[d]({clientX:E.clientX,clientY:E.clientY,target:a,rootEl:b});break}a=b}while(b=b.parentNode);i(s,"display","")}},_onTouchMove:function(a){if(D){var b=a.touches?a.touches[0]:a,c=b.clientX-D.clientX,d=b.clientY-D.clientY,e=a.touches?"translate3d("+c+"px,"+d+"px,0)":"translate("+c+"px,"+d+"px)";E=b,i(s,"webkitTransform",e),i(s,"mozTransform",e),i(s,"msTransform",e),i(s,"transform",e),a.preventDefault()}},_onDragStart:function(a,b){var c=a.dataTransfer,d=this.options;if(this._offUpEvents(),"clone"==C.pull&&(t=r.cloneNode(!0),i(t,"display","none"),u.insertBefore(t,r)),b){var e,g=r.getBoundingClientRect(),h=i(r);s=r.cloneNode(!0),i(s,"top",g.top-K(h.marginTop,10)),i(s,"left",g.left-K(h.marginLeft,10)),i(s,"width",g.width),i(s,"height",g.height),i(s,"opacity","0.8"),i(s,"position","fixed"),i(s,"zIndex","100000"),u.appendChild(s),e=s.getBoundingClientRect(),i(s,"width",2*g.width-e.width),i(s,"height",2*g.height-e.height),"touch"===b?(f(J,"touchmove",this._onTouchMove),f(J,"touchend",this._onDrop),f(J,"touchcancel",this._onDrop)):(f(J,"mousemove",this._onTouchMove),f(J,"mouseup",this._onDrop)),this._loopId=setInterval(this._emulateDragOver,150)}else c&&(c.effectAllowed="move",d.setData&&d.setData.call(this,c,r)),f(J,"drop",this);setTimeout(this._dragStarted,0)},_onDragOver:function(a){var c,e,f,g=this.el,h=this.options,j=h.group,k=j.put,n=C===j,o=h.sort;if(void 0!==a.preventDefault&&(a.preventDefault(),!h.dragoverBubble&&a.stopPropagation()),C&&!h.disabled&&(n?o||(f=!u.contains(r)):C.pull&&k&&(C.name===j.name||k.indexOf&&~k.indexOf(C.name)))&&(void 0===a.rootEl||a.rootEl===this.el)){if(R(a,h,this.el),M)return;if(c=d(a.target,h.draggable,g),e=r.getBoundingClientRect(),f)return b(!0),void(t||v?u.insertBefore(r,t||v):o||u.appendChild(r));if(0===g.children.length||g.children[0]===s||g===a.target&&(c=m(g,a))){if(c){if(c.animated)return;q=c.getBoundingClientRect()}b(n),g.appendChild(r),this._animate(e,r),c&&this._animate(q,c)}else if(c&&!c.animated&&c!==r&&void 0!==c.parentNode[H]){y!==c&&(y=c,z=i(c));var p,q=c.getBoundingClientRect(),w=q.right-q.left,x=q.bottom-q.top,A=/left|right|inline/.test(z.cssFloat+z.display),B=c.offsetWidth>r.offsetWidth,D=c.offsetHeight>r.offsetHeight,E=(A?(a.clientX-q.left)/w:(a.clientY-q.top)/x)>.5,F=c.nextElementSibling;M=!0,setTimeout(l,30),b(n),p=A?c.previousElementSibling===r&&!B||E&&B:F!==r&&!D||E&&D,p&&!F?g.appendChild(r):c.parentNode.insertBefore(r,p?F:c),this._animate(e,r),this._animate(q,c)}}},_animate:function(a,b){var c=this.options.animation;if(c){var d=b.getBoundingClientRect();i(b,"transition","none"),i(b,"transform","translate3d("+(a.left-d.left)+"px,"+(a.top-d.top)+"px,0)"),b.offsetWidth,i(b,"transition","all "+c+"ms"),i(b,"transform","translate3d(0,0,0)"),clearTimeout(b.animated),b.animated=setTimeout(function(){i(b,"transition",""),i(b,"transform",""),b.animated=!1},c)}},_offUpEvents:function(){var a=this.el.ownerDocument;g(J,"touchmove",this._onTouchMove),g(a,"mouseup",this._onDrop),g(a,"touchend",this._onDrop),g(a,"touchcancel",this._onDrop)},_onDrop:function(b){var c=this.el,d=this.options;clearInterval(this._loopId),clearInterval(F.pid),clearTimeout(this.dragStartTimer),g(J,"drop",this),g(J,"mousemove",this._onTouchMove),g(c,"dragstart",this._onDragStart),this._offUpEvents(),b&&(b.preventDefault(),!d.dropBubble&&b.stopPropagation(),s&&s.parentNode.removeChild(s),r&&(g(r,"dragend",this),k(r),h(r,this.options.ghostClass,!1),u!==r.parentNode?(B=o(r),N(null,r.parentNode,"sort",r,u,A,B),N(this,u,"sort",r,u,A,B),N(null,r.parentNode,"add",r,u,A,B),N(this,u,"remove",r,u,A,B)):(t&&t.parentNode.removeChild(t),r.nextSibling!==v&&(B=o(r),N(this,u,"update",r,u,A,B),N(this,u,"sort",r,u,A,B))),a.active&&N(this,u,"end",r,u,A,B)),u=r=s=v=t=w=x=D=E=y=z=C=a.active=null,this.save())},handleEvent:function(a){var b=a.type;"dragover"===b||"dragenter"===b?r&&(this._onDragOver(a),e(a)):("drop"===b||"dragend"===b)&&this._onDrop(a)},toArray:function(){for(var a,b=[],c=this.el.children,e=0,f=c.length,g=this.options;f>e;e++)a=c[e],d(a,g.draggable,this.el)&&b.push(a.getAttribute(g.dataIdAttr)||n(a));return b},sort:function(a){var b={},c=this.el;this.toArray().forEach(function(a,e){var f=c.children[e];d(f,this.options.draggable,c)&&(b[a]=f)},this),a.forEach(function(a){b[a]&&(c.removeChild(b[a]),c.appendChild(b[a]))})},save:function(){var a=this.options.store;a&&a.set(this)},closest:function(a,b){return d(a,b||this.options.draggable,this.el)},option:function(a,b){var c=this.options;return void 0===b?c[a]:void(c[a]=b)},destroy:function(){var a=this.el;a[H]=null,g(a,"mousedown",this._onTapStart),g(a,"touchstart",this._onTapStart),g(a,"dragover",this),g(a,"dragenter",this),Array.prototype.forEach.call(a.querySelectorAll("[draggable]"),function(a){a.removeAttribute("draggable")}),Q.splice(Q.indexOf(this._onDragOver),1),this._onDrop(),this.el=a=null}},a.utils={on:f,off:g,css:i,find:j,bind:c,is:function(a,b){return!!d(a,b,a)},extend:q,throttle:p,closest:d,toggleClass:h,index:o},a.version="1.2.0",a.create=function(b,c){return new a(b,c)},a});
/* Zepto v1.1.6 - zepto event ajax form ie - zeptojs.com/license */
var Zepto=function(){function L(t){return null==t?String(t):j[S.call(t)]||"object"}function Z(t){return"function"==L(t)}function _(t){return null!=t&&t==t.window}function $(t){return null!=t&&t.nodeType==t.DOCUMENT_NODE}function D(t){return"object"==L(t)}function M(t){return D(t)&&!_(t)&&Object.getPrototypeOf(t)==Object.prototype}function R(t){return"number"==typeof t.length}function k(t){return s.call(t,function(t){return null!=t})}function z(t){return t.length>0?n.fn.concat.apply([],t):t}function F(t){return t.replace(/::/g,"/").replace(/([A-Z]+)([A-Z][a-z])/g,"$1_$2").replace(/([a-z\d])([A-Z])/g,"$1_$2").replace(/_/g,"-").toLowerCase()}function q(t){return t in f?f[t]:f[t]=new RegExp("(^|\\s)"+t+"(\\s|$)")}function H(t,e){return"number"!=typeof e||c[F(t)]?e:e+"px"}function I(t){var e,n;return u[t]||(e=a.createElement(t),a.body.appendChild(e),n=getComputedStyle(e,"").getPropertyValue("display"),e.parentNode.removeChild(e),"none"==n&&(n="block"),u[t]=n),u[t]}function V(t){return"children"in t?o.call(t.children):n.map(t.childNodes,function(t){return 1==t.nodeType?t:void 0})}function B(n,i,r){for(e in i)r&&(M(i[e])||A(i[e]))?(M(i[e])&&!M(n[e])&&(n[e]={}),A(i[e])&&!A(n[e])&&(n[e]=[]),B(n[e],i[e],r)):i[e]!==t&&(n[e]=i[e])}function U(t,e){return null==e?n(t):n(t).filter(e)}function J(t,e,n,i){return Z(e)?e.call(t,n,i):e}function X(t,e,n){null==n?t.removeAttribute(e):t.setAttribute(e,n)}function W(e,n){var i=e.className||"",r=i&&i.baseVal!==t;return n===t?r?i.baseVal:i:void(r?i.baseVal=n:e.className=n)}function Y(t){try{return t?"true"==t||("false"==t?!1:"null"==t?null:+t+""==t?+t:/^[\[\{]/.test(t)?n.parseJSON(t):t):t}catch(e){return t}}function G(t,e){e(t);for(var n=0,i=t.childNodes.length;i>n;n++)G(t.childNodes[n],e)}var t,e,n,i,C,N,r=[],o=r.slice,s=r.filter,a=window.document,u={},f={},c={"column-count":1,columns:1,"font-weight":1,"line-height":1,opacity:1,"z-index":1,zoom:1},l=/^\s*<(\w+|!)[^>]*>/,h=/^<(\w+)\s*\/?>(?:<\/\1>|)$/,p=/<(?!area|br|col|embed|hr|img|input|link|meta|param)(([\w:]+)[^>]*)\/>/gi,d=/^(?:body|html)$/i,m=/([A-Z])/g,g=["val","css","html","text","data","width","height","offset"],v=["after","prepend","before","append"],y=a.createElement("table"),x=a.createElement("tr"),b={tr:a.createElement("tbody"),tbody:y,thead:y,tfoot:y,td:x,th:x,"*":a.createElement("div")},w=/complete|loaded|interactive/,E=/^[\w-]*$/,j={},S=j.toString,T={},O=a.createElement("div"),P={tabindex:"tabIndex",readonly:"readOnly","for":"htmlFor","class":"className",maxlength:"maxLength",cellspacing:"cellSpacing",cellpadding:"cellPadding",rowspan:"rowSpan",colspan:"colSpan",usemap:"useMap",frameborder:"frameBorder",contenteditable:"contentEditable"},A=Array.isArray||function(t){return t instanceof Array};return T.matches=function(t,e){if(!e||!t||1!==t.nodeType)return!1;var n=t.webkitMatchesSelector||t.mozMatchesSelector||t.oMatchesSelector||t.matchesSelector;if(n)return n.call(t,e);var i,r=t.parentNode,o=!r;return o&&(r=O).appendChild(t),i=~T.qsa(r,e).indexOf(t),o&&O.removeChild(t),i},C=function(t){return t.replace(/-+(.)?/g,function(t,e){return e?e.toUpperCase():""})},N=function(t){return s.call(t,function(e,n){return t.indexOf(e)==n})},T.fragment=function(e,i,r){var s,u,f;return h.test(e)&&(s=n(a.createElement(RegExp.$1))),s||(e.replace&&(e=e.replace(p,"<$1></$2>")),i===t&&(i=l.test(e)&&RegExp.$1),i in b||(i="*"),f=b[i],f.innerHTML=""+e,s=n.each(o.call(f.childNodes),function(){f.removeChild(this)})),M(r)&&(u=n(s),n.each(r,function(t,e){g.indexOf(t)>-1?u[t](e):u.attr(t,e)})),s},T.Z=function(t,e){return t=t||[],t.__proto__=n.fn,t.selector=e||"",t},T.isZ=function(t){return t instanceof T.Z},T.init=function(e,i){var r;if(!e)return T.Z();if("string"==typeof e)if(e=e.trim(),"<"==e[0]&&l.test(e))r=T.fragment(e,RegExp.$1,i),e=null;else{if(i!==t)return n(i).find(e);r=T.qsa(a,e)}else{if(Z(e))return n(a).ready(e);if(T.isZ(e))return e;if(A(e))r=k(e);else if(D(e))r=[e],e=null;else if(l.test(e))r=T.fragment(e.trim(),RegExp.$1,i),e=null;else{if(i!==t)return n(i).find(e);r=T.qsa(a,e)}}return T.Z(r,e)},n=function(t,e){return T.init(t,e)},n.extend=function(t){var e,n=o.call(arguments,1);return"boolean"==typeof t&&(e=t,t=n.shift()),n.forEach(function(n){B(t,n,e)}),t},T.qsa=function(t,e){var n,i="#"==e[0],r=!i&&"."==e[0],s=i||r?e.slice(1):e,a=E.test(s);return $(t)&&a&&i?(n=t.getElementById(s))?[n]:[]:1!==t.nodeType&&9!==t.nodeType?[]:o.call(a&&!i?r?t.getElementsByClassName(s):t.getElementsByTagName(e):t.querySelectorAll(e))},n.contains=a.documentElement.contains?function(t,e){return t!==e&&t.contains(e)}:function(t,e){for(;e&&(e=e.parentNode);)if(e===t)return!0;return!1},n.type=L,n.isFunction=Z,n.isWindow=_,n.isArray=A,n.isPlainObject=M,n.isEmptyObject=function(t){var e;for(e in t)return!1;return!0},n.inArray=function(t,e,n){return r.indexOf.call(e,t,n)},n.camelCase=C,n.trim=function(t){return null==t?"":String.prototype.trim.call(t)},n.uuid=0,n.support={},n.expr={},n.map=function(t,e){var n,r,o,i=[];if(R(t))for(r=0;r<t.length;r++)n=e(t[r],r),null!=n&&i.push(n);else for(o in t)n=e(t[o],o),null!=n&&i.push(n);return z(i)},n.each=function(t,e){var n,i;if(R(t)){for(n=0;n<t.length;n++)if(e.call(t[n],n,t[n])===!1)return t}else for(i in t)if(e.call(t[i],i,t[i])===!1)return t;return t},n.grep=function(t,e){return s.call(t,e)},window.JSON&&(n.parseJSON=JSON.parse),n.each("Boolean Number String Function Array Date RegExp Object Error".split(" "),function(t,e){j["[object "+e+"]"]=e.toLowerCase()}),n.fn={forEach:r.forEach,reduce:r.reduce,push:r.push,sort:r.sort,indexOf:r.indexOf,concat:r.concat,map:function(t){return n(n.map(this,function(e,n){return t.call(e,n,e)}))},slice:function(){return n(o.apply(this,arguments))},ready:function(t){return w.test(a.readyState)&&a.body?t(n):a.addEventListener("DOMContentLoaded",function(){t(n)},!1),this},get:function(e){return e===t?o.call(this):this[e>=0?e:e+this.length]},toArray:function(){return this.get()},size:function(){return this.length},remove:function(){return this.each(function(){null!=this.parentNode&&this.parentNode.removeChild(this)})},each:function(t){return r.every.call(this,function(e,n){return t.call(e,n,e)!==!1}),this},filter:function(t){return Z(t)?this.not(this.not(t)):n(s.call(this,function(e){return T.matches(e,t)}))},add:function(t,e){return n(N(this.concat(n(t,e))))},is:function(t){return this.length>0&&T.matches(this[0],t)},not:function(e){var i=[];if(Z(e)&&e.call!==t)this.each(function(t){e.call(this,t)||i.push(this)});else{var r="string"==typeof e?this.filter(e):R(e)&&Z(e.item)?o.call(e):n(e);this.forEach(function(t){r.indexOf(t)<0&&i.push(t)})}return n(i)},has:function(t){return this.filter(function(){return D(t)?n.contains(this,t):n(this).find(t).size()})},eq:function(t){return-1===t?this.slice(t):this.slice(t,+t+1)},first:function(){var t=this[0];return t&&!D(t)?t:n(t)},last:function(){var t=this[this.length-1];return t&&!D(t)?t:n(t)},find:function(t){var e,i=this;return e=t?"object"==typeof t?n(t).filter(function(){var t=this;return r.some.call(i,function(e){return n.contains(e,t)})}):1==this.length?n(T.qsa(this[0],t)):this.map(function(){return T.qsa(this,t)}):n()},closest:function(t,e){var i=this[0],r=!1;for("object"==typeof t&&(r=n(t));i&&!(r?r.indexOf(i)>=0:T.matches(i,t));)i=i!==e&&!$(i)&&i.parentNode;return n(i)},parents:function(t){for(var e=[],i=this;i.length>0;)i=n.map(i,function(t){return(t=t.parentNode)&&!$(t)&&e.indexOf(t)<0?(e.push(t),t):void 0});return U(e,t)},parent:function(t){return U(N(this.pluck("parentNode")),t)},children:function(t){return U(this.map(function(){return V(this)}),t)},contents:function(){return this.map(function(){return o.call(this.childNodes)})},siblings:function(t){return U(this.map(function(t,e){return s.call(V(e.parentNode),function(t){return t!==e})}),t)},empty:function(){return this.each(function(){this.innerHTML=""})},pluck:function(t){return n.map(this,function(e){return e[t]})},show:function(){return this.each(function(){"none"==this.style.display&&(this.style.display=""),"none"==getComputedStyle(this,"").getPropertyValue("display")&&(this.style.display=I(this.nodeName))})},replaceWith:function(t){return this.before(t).remove()},wrap:function(t){var e=Z(t);if(this[0]&&!e)var i=n(t).get(0),r=i.parentNode||this.length>1;return this.each(function(o){n(this).wrapAll(e?t.call(this,o):r?i.cloneNode(!0):i)})},wrapAll:function(t){if(this[0]){n(this[0]).before(t=n(t));for(var e;(e=t.children()).length;)t=e.first();n(t).append(this)}return this},wrapInner:function(t){var e=Z(t);return this.each(function(i){var r=n(this),o=r.contents(),s=e?t.call(this,i):t;o.length?o.wrapAll(s):r.append(s)})},unwrap:function(){return this.parent().each(function(){n(this).replaceWith(n(this).children())}),this},clone:function(){return this.map(function(){return this.cloneNode(!0)})},hide:function(){return this.css("display","none")},toggle:function(e){return this.each(function(){var i=n(this);(e===t?"none"==i.css("display"):e)?i.show():i.hide()})},prev:function(t){return n(this.pluck("previousElementSibling")).filter(t||"*")},next:function(t){return n(this.pluck("nextElementSibling")).filter(t||"*")},html:function(t){return 0 in arguments?this.each(function(e){var i=this.innerHTML;n(this).empty().append(J(this,t,e,i))}):0 in this?this[0].innerHTML:null},text:function(t){return 0 in arguments?this.each(function(e){var n=J(this,t,e,this.textContent);this.textContent=null==n?"":""+n}):0 in this?this[0].textContent:null},attr:function(n,i){var r;return"string"!=typeof n||1 in arguments?this.each(function(t){if(1===this.nodeType)if(D(n))for(e in n)X(this,e,n[e]);else X(this,n,J(this,i,t,this.getAttribute(n)))}):this.length&&1===this[0].nodeType?!(r=this[0].getAttribute(n))&&n in this[0]?this[0][n]:r:t},removeAttr:function(t){return this.each(function(){1===this.nodeType&&t.split(" ").forEach(function(t){X(this,t)},this)})},prop:function(t,e){return t=P[t]||t,1 in arguments?this.each(function(n){this[t]=J(this,e,n,this[t])}):this[0]&&this[0][t]},data:function(e,n){var i="data-"+e.replace(m,"-$1").toLowerCase(),r=1 in arguments?this.attr(i,n):this.attr(i);return null!==r?Y(r):t},val:function(t){return 0 in arguments?this.each(function(e){this.value=J(this,t,e,this.value)}):this[0]&&(this[0].multiple?n(this[0]).find("option").filter(function(){return this.selected}).pluck("value"):this[0].value)},offset:function(t){if(t)return this.each(function(e){var i=n(this),r=J(this,t,e,i.offset()),o=i.offsetParent().offset(),s={top:r.top-o.top,left:r.left-o.left};"static"==i.css("position")&&(s.position="relative"),i.css(s)});if(!this.length)return null;var e=this[0].getBoundingClientRect();return{left:e.left+window.pageXOffset,top:e.top+window.pageYOffset,width:Math.round(e.width),height:Math.round(e.height)}},css:function(t,i){if(arguments.length<2){var r,o=this[0];if(!o)return;if(r=getComputedStyle(o,""),"string"==typeof t)return o.style[C(t)]||r.getPropertyValue(t);if(A(t)){var s={};return n.each(t,function(t,e){s[e]=o.style[C(e)]||r.getPropertyValue(e)}),s}}var a="";if("string"==L(t))i||0===i?a=F(t)+":"+H(t,i):this.each(function(){this.style.removeProperty(F(t))});else for(e in t)t[e]||0===t[e]?a+=F(e)+":"+H(e,t[e])+";":this.each(function(){this.style.removeProperty(F(e))});return this.each(function(){this.style.cssText+=";"+a})},index:function(t){return t?this.indexOf(n(t)[0]):this.parent().children().indexOf(this[0])},hasClass:function(t){return t?r.some.call(this,function(t){return this.test(W(t))},q(t)):!1},addClass:function(t){return t?this.each(function(e){if("className"in this){i=[];var r=W(this),o=J(this,t,e,r);o.split(/\s+/g).forEach(function(t){n(this).hasClass(t)||i.push(t)},this),i.length&&W(this,r+(r?" ":"")+i.join(" "))}}):this},removeClass:function(e){return this.each(function(n){if("className"in this){if(e===t)return W(this,"");i=W(this),J(this,e,n,i).split(/\s+/g).forEach(function(t){i=i.replace(q(t)," ")}),W(this,i.trim())}})},toggleClass:function(e,i){return e?this.each(function(r){var o=n(this),s=J(this,e,r,W(this));s.split(/\s+/g).forEach(function(e){(i===t?!o.hasClass(e):i)?o.addClass(e):o.removeClass(e)})}):this},scrollTop:function(e){if(this.length){var n="scrollTop"in this[0];return e===t?n?this[0].scrollTop:this[0].pageYOffset:this.each(n?function(){this.scrollTop=e}:function(){this.scrollTo(this.scrollX,e)})}},scrollLeft:function(e){if(this.length){var n="scrollLeft"in this[0];return e===t?n?this[0].scrollLeft:this[0].pageXOffset:this.each(n?function(){this.scrollLeft=e}:function(){this.scrollTo(e,this.scrollY)})}},position:function(){if(this.length){var t=this[0],e=this.offsetParent(),i=this.offset(),r=d.test(e[0].nodeName)?{top:0,left:0}:e.offset();return i.top-=parseFloat(n(t).css("margin-top"))||0,i.left-=parseFloat(n(t).css("margin-left"))||0,r.top+=parseFloat(n(e[0]).css("border-top-width"))||0,r.left+=parseFloat(n(e[0]).css("border-left-width"))||0,{top:i.top-r.top,left:i.left-r.left}}},offsetParent:function(){return this.map(function(){for(var t=this.offsetParent||a.body;t&&!d.test(t.nodeName)&&"static"==n(t).css("position");)t=t.offsetParent;return t})}},n.fn.detach=n.fn.remove,["width","height"].forEach(function(e){var i=e.replace(/./,function(t){return t[0].toUpperCase()});n.fn[e]=function(r){var o,s=this[0];return r===t?_(s)?s["inner"+i]:$(s)?s.documentElement["scroll"+i]:(o=this.offset())&&o[e]:this.each(function(t){s=n(this),s.css(e,J(this,r,t,s[e]()))})}}),v.forEach(function(t,e){var i=e%2;n.fn[t]=function(){var t,o,r=n.map(arguments,function(e){return t=L(e),"object"==t||"array"==t||null==e?e:T.fragment(e)}),s=this.length>1;return r.length<1?this:this.each(function(t,u){o=i?u:u.parentNode,u=0==e?u.nextSibling:1==e?u.firstChild:2==e?u:null;var f=n.contains(a.documentElement,o);r.forEach(function(t){if(s)t=t.cloneNode(!0);else if(!o)return n(t).remove();o.insertBefore(t,u),f&&G(t,function(t){null==t.nodeName||"SCRIPT"!==t.nodeName.toUpperCase()||t.type&&"text/javascript"!==t.type||t.src||window.eval.call(window,t.innerHTML)})})})},n.fn[i?t+"To":"insert"+(e?"Before":"After")]=function(e){return n(e)[t](this),this}}),T.Z.prototype=n.fn,T.uniq=N,T.deserializeValue=Y,n.zepto=T,n}();window.Zepto=Zepto,void 0===window.$&&(window.$=Zepto),function(t){function l(t){return t._zid||(t._zid=e++)}function h(t,e,n,i){if(e=p(e),e.ns)var r=d(e.ns);return(s[l(t)]||[]).filter(function(t){return!(!t||e.e&&t.e!=e.e||e.ns&&!r.test(t.ns)||n&&l(t.fn)!==l(n)||i&&t.sel!=i)})}function p(t){var e=(""+t).split(".");return{e:e[0],ns:e.slice(1).sort().join(" ")}}function d(t){return new RegExp("(?:^| )"+t.replace(" "," .* ?")+"(?: |$)")}function m(t,e){return t.del&&!u&&t.e in f||!!e}function g(t){return c[t]||u&&f[t]||t}function v(e,i,r,o,a,u,f){var h=l(e),d=s[h]||(s[h]=[]);i.split(/\s/).forEach(function(i){if("ready"==i)return t(document).ready(r);var s=p(i);s.fn=r,s.sel=a,s.e in c&&(r=function(e){var n=e.relatedTarget;return!n||n!==this&&!t.contains(this,n)?s.fn.apply(this,arguments):void 0}),s.del=u;var l=u||r;s.proxy=function(t){if(t=j(t),!t.isImmediatePropagationStopped()){t.data=o;var i=l.apply(e,t._args==n?[t]:[t].concat(t._args));return i===!1&&(t.preventDefault(),t.stopPropagation()),i}},s.i=d.length,d.push(s),"addEventListener"in e&&e.addEventListener(g(s.e),s.proxy,m(s,f))})}function y(t,e,n,i,r){var o=l(t);(e||"").split(/\s/).forEach(function(e){h(t,e,n,i).forEach(function(e){delete s[o][e.i],"removeEventListener"in t&&t.removeEventListener(g(e.e),e.proxy,m(e,r))})})}function j(e,i){return(i||!e.isDefaultPrevented)&&(i||(i=e),t.each(E,function(t,n){var r=i[t];e[t]=function(){return this[n]=x,r&&r.apply(i,arguments)},e[n]=b}),(i.defaultPrevented!==n?i.defaultPrevented:"returnValue"in i?i.returnValue===!1:i.getPreventDefault&&i.getPreventDefault())&&(e.isDefaultPrevented=x)),e}function S(t){var e,i={originalEvent:t};for(e in t)w.test(e)||t[e]===n||(i[e]=t[e]);return j(i,t)}var n,e=1,i=Array.prototype.slice,r=t.isFunction,o=function(t){return"string"==typeof t},s={},a={},u="onfocusin"in window,f={focus:"focusin",blur:"focusout"},c={mouseenter:"mouseover",mouseleave:"mouseout"};a.click=a.mousedown=a.mouseup=a.mousemove="MouseEvents",t.event={add:v,remove:y},t.proxy=function(e,n){var s=2 in arguments&&i.call(arguments,2);if(r(e)){var a=function(){return e.apply(n,s?s.concat(i.call(arguments)):arguments)};return a._zid=l(e),a}if(o(n))return s?(s.unshift(e[n],e),t.proxy.apply(null,s)):t.proxy(e[n],e);throw new TypeError("expected function")},t.fn.bind=function(t,e,n){return this.on(t,e,n)},t.fn.unbind=function(t,e){return this.off(t,e)},t.fn.one=function(t,e,n,i){return this.on(t,e,n,i,1)};var x=function(){return!0},b=function(){return!1},w=/^([A-Z]|returnValue$|layer[XY]$)/,E={preventDefault:"isDefaultPrevented",stopImmediatePropagation:"isImmediatePropagationStopped",stopPropagation:"isPropagationStopped"};t.fn.delegate=function(t,e,n){return this.on(e,t,n)},t.fn.undelegate=function(t,e,n){return this.off(e,t,n)},t.fn.live=function(e,n){return t(document.body).delegate(this.selector,e,n),this},t.fn.die=function(e,n){return t(document.body).undelegate(this.selector,e,n),this},t.fn.on=function(e,s,a,u,f){var c,l,h=this;return e&&!o(e)?(t.each(e,function(t,e){h.on(t,s,a,e,f)}),h):(o(s)||r(u)||u===!1||(u=a,a=s,s=n),(r(a)||a===!1)&&(u=a,a=n),u===!1&&(u=b),h.each(function(n,r){f&&(c=function(t){return y(r,t.type,u),u.apply(this,arguments)}),s&&(l=function(e){var n,o=t(e.target).closest(s,r).get(0);return o&&o!==r?(n=t.extend(S(e),{currentTarget:o,liveFired:r}),(c||u).apply(o,[n].concat(i.call(arguments,1)))):void 0}),v(r,e,u,a,s,l||c)}))},t.fn.off=function(e,i,s){var a=this;return e&&!o(e)?(t.each(e,function(t,e){a.off(t,i,e)}),a):(o(i)||r(s)||s===!1||(s=i,i=n),s===!1&&(s=b),a.each(function(){y(this,e,s,i)}))},t.fn.trigger=function(e,n){return e=o(e)||t.isPlainObject(e)?t.Event(e):j(e),e._args=n,this.each(function(){e.type in f&&"function"==typeof this[e.type]?this[e.type]():"dispatchEvent"in this?this.dispatchEvent(e):t(this).triggerHandler(e,n)})},t.fn.triggerHandler=function(e,n){var i,r;return this.each(function(s,a){i=S(o(e)?t.Event(e):e),i._args=n,i.target=a,t.each(h(a,e.type||e),function(t,e){return r=e.proxy(i),i.isImmediatePropagationStopped()?!1:void 0})}),r},"focusin focusout focus blur load resize scroll unload click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select keydown keypress keyup error".split(" ").forEach(function(e){t.fn[e]=function(t){return 0 in arguments?this.bind(e,t):this.trigger(e)}}),t.Event=function(t,e){o(t)||(e=t,t=e.type);var n=document.createEvent(a[t]||"Events"),i=!0;if(e)for(var r in e)"bubbles"==r?i=!!e[r]:n[r]=e[r];return n.initEvent(t,i,!0),j(n)}}(Zepto),function(t){function h(e,n,i){var r=t.Event(n);return t(e).trigger(r,i),!r.isDefaultPrevented()}function p(t,e,i,r){return t.global?h(e||n,i,r):void 0}function d(e){e.global&&0===t.active++&&p(e,null,"ajaxStart")}function m(e){e.global&&!--t.active&&p(e,null,"ajaxStop")}function g(t,e){var n=e.context;return e.beforeSend.call(n,t,e)===!1||p(e,n,"ajaxBeforeSend",[t,e])===!1?!1:void p(e,n,"ajaxSend",[t,e])}function v(t,e,n,i){var r=n.context,o="success";n.success.call(r,t,o,e),i&&i.resolveWith(r,[t,o,e]),p(n,r,"ajaxSuccess",[e,n,t]),x(o,e,n)}function y(t,e,n,i,r){var o=i.context;i.error.call(o,n,e,t),r&&r.rejectWith(o,[n,e,t]),p(i,o,"ajaxError",[n,i,t||e]),x(e,n,i)}function x(t,e,n){var i=n.context;n.complete.call(i,e,t),p(n,i,"ajaxComplete",[e,n]),m(n)}function b(){}function w(t){return t&&(t=t.split(";",2)[0]),t&&(t==f?"html":t==u?"json":s.test(t)?"script":a.test(t)&&"xml")||"text"}function E(t,e){return""==e?t:(t+"&"+e).replace(/[&?]{1,2}/,"?")}function j(e){e.processData&&e.data&&"string"!=t.type(e.data)&&(e.data=t.param(e.data,e.traditional)),!e.data||e.type&&"GET"!=e.type.toUpperCase()||(e.url=E(e.url,e.data),e.data=void 0)}function S(e,n,i,r){return t.isFunction(n)&&(r=i,i=n,n=void 0),t.isFunction(i)||(r=i,i=void 0),{url:e,data:n,success:i,dataType:r}}function C(e,n,i,r){var o,s=t.isArray(n),a=t.isPlainObject(n);t.each(n,function(n,u){o=t.type(u),r&&(n=i?r:r+"["+(a||"object"==o||"array"==o?n:"")+"]"),!r&&s?e.add(u.name,u.value):"array"==o||!i&&"object"==o?C(e,u,i,n):e.add(n,u)})}var i,r,e=0,n=window.document,o=/<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi,s=/^(?:text|application)\/javascript/i,a=/^(?:text|application)\/xml/i,u="application/json",f="text/html",c=/^\s*$/,l=n.createElement("a");l.href=window.location.href,t.active=0,t.ajaxJSONP=function(i,r){if(!("type"in i))return t.ajax(i);var f,h,o=i.jsonpCallback,s=(t.isFunction(o)?o():o)||"jsonp"+ ++e,a=n.createElement("script"),u=window[s],c=function(e){t(a).triggerHandler("error",e||"abort")},l={abort:c};return r&&r.promise(l),t(a).on("load error",function(e,n){clearTimeout(h),t(a).off().remove(),"error"!=e.type&&f?v(f[0],l,i,r):y(null,n||"error",l,i,r),window[s]=u,f&&t.isFunction(u)&&u(f[0]),u=f=void 0}),g(l,i)===!1?(c("abort"),l):(window[s]=function(){f=arguments},a.src=i.url.replace(/\?(.+)=\?/,"?$1="+s),n.head.appendChild(a),i.timeout>0&&(h=setTimeout(function(){c("timeout")},i.timeout)),l)},t.ajaxSettings={type:"GET",beforeSend:b,success:b,error:b,complete:b,context:null,global:!0,xhr:function(){return new window.XMLHttpRequest},accepts:{script:"text/javascript, application/javascript, application/x-javascript",json:u,xml:"application/xml, text/xml",html:f,text:"text/plain"},crossDomain:!1,timeout:0,processData:!0,cache:!0},t.ajax=function(e){var a,o=t.extend({},e||{}),s=t.Deferred&&t.Deferred();for(i in t.ajaxSettings)void 0===o[i]&&(o[i]=t.ajaxSettings[i]);d(o),o.crossDomain||(a=n.createElement("a"),a.href=o.url,a.href=a.href,o.crossDomain=l.protocol+"//"+l.host!=a.protocol+"//"+a.host),o.url||(o.url=window.location.toString()),j(o);var u=o.dataType,f=/\?.+=\?/.test(o.url);if(f&&(u="jsonp"),o.cache!==!1&&(e&&e.cache===!0||"script"!=u&&"jsonp"!=u)||(o.url=E(o.url,"_="+Date.now())),"jsonp"==u)return f||(o.url=E(o.url,o.jsonp?o.jsonp+"=?":o.jsonp===!1?"":"callback=?")),t.ajaxJSONP(o,s);var C,h=o.accepts[u],p={},m=function(t,e){p[t.toLowerCase()]=[t,e]},x=/^([\w-]+:)\/\//.test(o.url)?RegExp.$1:window.location.protocol,S=o.xhr(),T=S.setRequestHeader;if(s&&s.promise(S),o.crossDomain||m("X-Requested-With","XMLHttpRequest"),m("Accept",h||"*/*"),(h=o.mimeType||h)&&(h.indexOf(",")>-1&&(h=h.split(",",2)[0]),S.overrideMimeType&&S.overrideMimeType(h)),(o.contentType||o.contentType!==!1&&o.data&&"GET"!=o.type.toUpperCase())&&m("Content-Type",o.contentType||"application/x-www-form-urlencoded"),o.headers)for(r in o.headers)m(r,o.headers[r]);if(S.setRequestHeader=m,S.onreadystatechange=function(){if(4==S.readyState){S.onreadystatechange=b,clearTimeout(C);var e,n=!1;if(S.status>=200&&S.status<300||304==S.status||0==S.status&&"file:"==x){u=u||w(o.mimeType||S.getResponseHeader("content-type")),e=S.responseText;try{"script"==u?(1,eval)(e):"xml"==u?e=S.responseXML:"json"==u&&(e=c.test(e)?null:t.parseJSON(e))}catch(i){n=i}n?y(n,"parsererror",S,o,s):v(e,S,o,s)}else y(S.statusText||null,S.status?"error":"abort",S,o,s)}},g(S,o)===!1)return S.abort(),y(null,"abort",S,o,s),S;if(o.xhrFields)for(r in o.xhrFields)S[r]=o.xhrFields[r];var N="async"in o?o.async:!0;S.open(o.type,o.url,N,o.username,o.password);for(r in p)T.apply(S,p[r]);return o.timeout>0&&(C=setTimeout(function(){S.onreadystatechange=b,S.abort(),y(null,"timeout",S,o,s)},o.timeout)),S.send(o.data?o.data:null),S},t.get=function(){return t.ajax(S.apply(null,arguments))},t.post=function(){var e=S.apply(null,arguments);return e.type="POST",t.ajax(e)},t.getJSON=function(){var e=S.apply(null,arguments);return e.dataType="json",t.ajax(e)},t.fn.load=function(e,n,i){if(!this.length)return this;var a,r=this,s=e.split(/\s/),u=S(e,n,i),f=u.success;return s.length>1&&(u.url=s[0],a=s[1]),u.success=function(e){r.html(a?t("<div>").html(e.replace(o,"")).find(a):e),f&&f.apply(r,arguments)},t.ajax(u),this};var T=encodeURIComponent;t.param=function(e,n){var i=[];return i.add=function(e,n){t.isFunction(n)&&(n=n()),null==n&&(n=""),this.push(T(e)+"="+T(n))},C(i,e,n),i.join("&").replace(/%20/g,"+")}}(Zepto),function(t){t.fn.serializeArray=function(){var e,n,i=[],r=function(t){return t.forEach?t.forEach(r):void i.push({name:e,value:t})};return this[0]&&t.each(this[0].elements,function(i,o){n=o.type,e=o.name,e&&"fieldset"!=o.nodeName.toLowerCase()&&!o.disabled&&"submit"!=n&&"reset"!=n&&"button"!=n&&"file"!=n&&("radio"!=n&&"checkbox"!=n||o.checked)&&r(t(o).val())}),i},t.fn.serialize=function(){var t=[];return this.serializeArray().forEach(function(e){t.push(encodeURIComponent(e.name)+"="+encodeURIComponent(e.value))}),t.join("&")},t.fn.submit=function(e){if(0 in arguments)this.bind("submit",e);else if(this.length){var n=t.Event("submit");this.eq(0).trigger(n),n.isDefaultPrevented()||this.get(0).submit()}return this}}(Zepto),function(t){"__proto__"in{}||t.extend(t.zepto,{Z:function(e,n){return e=e||[],t.extend(e,t.fn),e.selector=n||"",e.__Z=!0,e},isZ:function(e){return"array"===t.type(e)&&"__Z"in e}});try{getComputedStyle(void 0)}catch(e){var n=getComputedStyle;window.getComputedStyle=function(t){try{return n(t)}catch(e){return null}}}}(Zepto);
'use strict';

(function($, Sortable) {

  var ENUMS = {
    COLOR : {
      EMPTY: 'transparent'
    }
  }

  $.extend($.fn, {
    itemToggle: function() {
      this.each(function() {
        var $checkbox = $(this);
        var item = $checkbox.parent();

        var $injectedCheckbox = $('<div class="item-styled-toggle-wrapper">'
                                 + '<div class="item-styled-toggle">'
                                   + '<div class="item-styled-toggle-bar"></div>'
                                   + '<div class="item-styled-toggle-bar"></div>'
                                   + '<div class="item-styled-toggle-bar"></div>'
                                 + '</div>'
                               + '</div>');

        item.append($injectedCheckbox);
      });
    },

    itemCheckbox: function() {
      this.each(function() {
        var $checkbox = $(this);
        var item = $checkbox.parent();

        var $injectedCheckbox = $('<div class="item-styled-checkbox"></div>');

        item.append($injectedCheckbox);
      });
    },

    itemSelect: function() {
      this.each(function() {
        var $select = $(this);
        var $item = $select.parent();

        $item.append('<div class="select-triangle"></div>');
      });
    },

    itemDate: function() {
      this.each(function() {
        var $date = $(this);
        var $item = $date.parent();

        var $injectedDate = $('<div class="item-styled-date"></div>');
        updateDate();

        $item.append($injectedDate);

        $date.change(function() {
          updateDate();
        });

        function updateDate() {
          $injectedDate.text($date.val());
        }
      });
    },

    itemTime: function() {
      this.each(function() {
        var $time = $(this);
        var $item = $time.parent();

        var $injectedTime = $('<div class="item-styled-time"></div>');
        updateTime();

        $item.append($injectedTime);

        $time.change(function() {
          updateTime();
        });

        function updateTime() {
          $injectedTime.text($time.val());
        }
      });
    },

    itemRadio: function() {
      this.each(function() {
        var $radio = $(this);
        var $item  = $radio.parent();

        var $injectedRadio = $('<div class="item-styled-radio"></div>');

        $item.append($injectedRadio);
      });
    },

    itemColor: function(options){

      var options = $.extend({}, {
        sunny: false
      }, options || {});

      var layout = [
        [false    , false    , '#55FF00', '#AAFF55', false    , '#FFFF55', '#FFFFAA', false    , false    ],
        [false    , '#AAFFAA', '#55FF55', '#00FF00', '#AAFF00', '#FFFF00', '#FFAA55', '#FFAAAA', false    ],
        ['#55FFAA', '#00FF55', '#00AA00', '#55AA00', '#AAAA55', '#AAAA00', '#FFAA00', '#FF5500', '#FF5555'],
        ['#AAFFFF', '#00FFAA', '#00AA55', '#55AA55', '#005500', '#555500', '#AA5500', '#FF0000', '#FF0055'],
        [false    , '#55AAAA', '#00AAAA', '#005555', '#FFFFFF', '#000000', '#AA5555', '#AA0000', false    ],
        ['#55FFFF', '#00FFFF', '#00AAFF', '#0055AA', '#AAAAAA', '#555555', '#550000', '#AA0055', '#FF55AA'],
        ['#55AAFF', '#0055FF', '#0000FF', '#0000AA', '#000055', '#550055', '#AA00AA', '#FF00AA', '#FFAAFF'],
        [false    , '#5555AA', '#5555FF', '#5500FF', '#5500AA', '#AA00FF', '#FF00FF', '#FF55FF', false    ],
        [false    , false    , false    , '#AAAAFF', '#AA55FF', '#AA55AA', false    , false    , false    ],
      ];

      var mappingSunny = {'000000': '000000','000055': '001e41','0000aa': '004387',
                          '0000ff': '0068ca','005500': '2b4a2c','005555': '27514f',
                          '0055aa': '16638d','0055ff': '007dce','00aa00': '5e9860',
                          '00aa55': '5c9b72','00aaaa': '57a5a2','00aaff': '4cb4db',
                          '00ff00': '8ee391','00ff55': '8ee69e','00ffaa': '8aebc0',
                          '00ffff': '84f5f1','550000': '4a161b','550055': '482748',
                          '5500aa': '40488a','5500ff': '2f6bcc','555500': '564e36',
                          '555555': '545454','5555aa': '4f6790','5555ff': '4180d0',
                          '55aa00': '759a64','55aa55': '759d76','55aaaa': '71a6a4',
                          '55aaff': '69b5dd','55ff00': '9ee594','55ff55': '9de7a0',
                          '55ffaa': '9becc2','55ffff': '95f6f2','aa0000': '99353f',
                          'aa0055': '983e5a','aa00aa': '955694','aa00ff': '8f74d2',
                          'aa5500': '9d5b4d','aa5555': '9d6064','aa55aa': '9a7099',
                          'aa55ff': '9587d5','aaaa00': 'afa072','aaaa55': 'aea382',
                          'aaaaaa': 'ababab','ffffff': 'ffffff','aaaaff': 'a7bae2',
                          'aaff00': 'c9e89d','aaff55': 'c9eaa7','aaffaa': 'c7f0c8',
                          'aaffff': 'c3f9f7','ff0000': 'e35462','ff0055': 'e25874',
                          'ff00aa': 'e16aa3','ff00ff': 'de83dc','ff5500': 'e66e6b',
                          'ff5555': 'e6727c','ff55aa': 'e37fa7','ff55ff': 'e194df',
                          'ffaa00': 'f1aa86','ffaa55': 'f1ad93','ffaaaa': 'efb5b8',
                          'ffaaff': 'ecc3eb','ffff00': 'ffeeab','ffff55': 'fff1b5',
                          'ffffaa': 'fff6d3'};

      this.each(function() {
        var $color = $(this);
        var $item = $color.parent();
        var grid = '';
        var itemWidth = 100 / layout[0].length;
        var itemHeight = 100 / layout.length;
        var boxHeight = itemWidth * layout.length;

        for(var i = 0; i < layout.length; i++) {
          for(var j = 0; j < layout[i].length; j++) {

            var color = layout[i][j] || ENUMS.COLOR.EMPTY;
            var selectable = (color !== ENUMS.COLOR.EMPTY ? ' selectable' : '');

            var roundedTL = (i === 0 && j === 0)
                || i === 0 && !layout[i][j - 1]
                || !layout[i][j - 1] && !layout[i -1][j]
                    ? ' rounded-tl' : '';

            var roundedTR = i === 0 && !layout[i][j + 1]
                || !layout[i][j + 1] && !layout[i -1][j]
                    ? ' rounded-tr ' : '';

            var roundedBL = (i === layout.length - 1 && j === 0)
                || i === layout.length - 1 && !layout[i][j - 1]
                || !layout[i][j - 1] && !layout[i + 1][j]
                    ? ' rounded-bl' : '';

            var roundedBR = i === layout.length - 1 && !layout[i][j + 1]
                || !layout[i][j + 1] && !layout[i + 1][j]
                    ? ' rounded-br' : '';

            if(options.sunny && color !== ENUMS.COLOR.EMPTY) {
              color = '#' + mappingSunny[color.replace('#', '').toLowerCase()];
            }

            grid += '<i ' +
              'class="color-box ' + selectable + roundedTL +
                      roundedTR + roundedBL + roundedBR + '" ' +
              'data-value="' + color.replace(/^#/, '0x') + '" ' +
              'style="' +
                'width:' + itemWidth + '%; ' +
                'height:' + itemHeight + '%; ' +
                'background:' + color + ';">' +
            '</i>';
          }
        }

        var $injectedColor = $('<div class="item-styled-color">' +
          '<span class="value" style="background:' + $color.val().replace(/^0x/, '#') + '"></span>' +
          '<div ' +
              'style="padding-bottom:' + boxHeight + '%"' +
              'class="color-box-wrap">' +
            '<div class="color-box-container">' +
                grid +
            '</div>' +
          '</div>' +
        '</div>');
        $item.append($injectedColor);

        var $valueDisplay = $injectedColor.find('.value');

        $color.on('click', function(ev) {
          $item.find('.color-box-wrap').toggleClass('show');
        });

        $item.find('.color-box.selectable').on('click', function(ev) {
          ev.preventDefault();

          var value = $(this).data('value');
          $color.val(value);
          $valueDisplay.css('background-color', value.replace(/^0x/, '#'));
          $item.find('.color-box-wrap').removeClass('show');
        })

      });
    },

    tab: function() {
      this.each(function() {
        var $tab = $(this);

        $tab.click(function() {
          var $current = $(this);
          var name = $current.attr('name');

          $('a[name=' + name + ']').each(function(){
            $(this).removeClass('active');
          });

          $current.addClass('active');
        });
      });
    },

    itemSlider: function() {
      this.each(function() {
        var $slider = $(this);
        var name = $slider.attr('name');
        var $input = $('input[name=' + name + '][class=item-input]');

        $slider.on('input', function() {
          var $current = $(this);
          $input.val($current.val());
        });

        $input.change(function() {
          var $current = $(this);
          $slider.val($current.val());
        });
      });
    },

    itemDraggableList: function() {
      this.each(function() {
        var $handlebar = '<div class="item-draggable-handle">'
                        + '<div class="item-draggable-handle-bar"></div>'
                        + '<div class="item-draggable-handle-bar"></div>'
                        + '<div class="item-draggable-handle-bar"></div>'
                      + '</div>';

        $(this).children('label').append($handlebar);

        Sortable.create(this, {
          handle: '.item-draggable-handle'
        });
      });
    },

    itemDynamicList: function() {
      this.each(function() {
        var $list = $(this);

        $list.children('label').each(function() {
          var $deleteButton = $('<div class="delete-item"></div>');

          $deleteButton.click(function() {
            $(this).parent().remove();
          });

          $(this).append($deleteButton);
        });

        var $addButton = $('<div class="item add-item">Add one more...</div>');

        $list.append($addButton);

        $addButton.click(function() {
          var $inbox = $('<div class="item">'
                        + '<div class="item-input-wrapper">'
                          + '<input class="item-input" type="text" name="focus-box">'
                        + '</div>'
                      + '</div>');

          $inbox.insertBefore($list.children().last());

          var $input = $inbox.find('input');
          $input.focus();

          $input.keypress(function(e) {
            var key = e.which;
            if (key === 13) {
              stopEditing($input, $inbox);
            }
          });

          $input.focusout(function() {
            stopEditing($input, $inbox);
          });

          function stopEditing(input, inbox) {
            var text = input.val();
            inbox.text(text);

            var deletebutton = $('<div class="delete-item"></div>');

            deletebutton.click(function(){
              $(this).parent().remove();
            });

            inbox.append(deletebutton);
          }
        });
      });
    }
  });

  $(function() {
    $('.item-toggle').itemToggle();
    $('.item-checkbox').itemCheckbox();
    $('.item-select').itemSelect();
    $('.item-date').itemDate();
    $('.item-time').itemTime();
    $('.item-radio').itemRadio();
    $('.item-color-normal').itemColor({sunny: false});
    $('.item-color-sunny').itemColor({sunny: true});
    $('.tab-button').tab();
    $('.item-slider').itemSlider();
    $('.item-draggable-list').itemDraggableList();
    $('.item-dynamic-list').itemDynamicList();
  });
}(Zepto, Sortable));
