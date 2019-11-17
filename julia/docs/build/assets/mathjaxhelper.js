MathJax.Hub.Config({
  "tex2jax": {
    inlineMath: [['$','$'], ['\\(','\\)']],
    processEscapes: true
  }
});
MathJax.Hub.Config({
  config: ["MMLorHTML.js"],
  jax: [
    "input/TeX",
    "output/HTML-CSS",
    "output/NativeMML"
  ],
  extensions: [
    "MathMenu.js",
    "MathZoom.js",
    "TeX/AMSmath.js",
    "TeX/AMSsymbols.js",
    "TeX/autobold.js",
    "TeX/autoload-all.js"
  ]
});
MathJax.Hub.Config({
  TeX: { equationNumbers: { autoNumber: "AMS" } }
});
