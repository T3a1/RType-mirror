// Adds a button to every page of the documentation website, leading to the same
// page in the other language: English at the root, French under fr/.
(function () {
    "use strict";

    var PAGE_PREFIX = "md_docs_2";
    var FRENCH_PREFIX = "md_docs_2fr_2";

    function otherLanguageUrl(path) {
        var parts = path.split("/");
        var name = parts[parts.length - 1] || "index.html";
        var french = parts[parts.length - 2] === "fr";
        if (french) {
            if (name.indexOf(FRENCH_PREFIX) === 0) {
                return "../" + PAGE_PREFIX + name.slice(FRENCH_PREFIX.length);
            }
            return "../index.html";
        }
        if (name.indexOf(PAGE_PREFIX) === 0) {
            return "fr/" + FRENCH_PREFIX + name.slice(PAGE_PREFIX.length);
        }
        return "fr/index.html";
    }

    function addButton() {
        var french = window.location.pathname.indexOf("/fr/") !== -1;
        var link = document.createElement("a");
        link.href = otherLanguageUrl(window.location.pathname);
        link.textContent = french ? "English" : "Français";
        link.lang = french ? "en" : "fr";
        link.hreflang = link.lang;
        link.title = french ? "Read this page in English" : "Lire cette page en français";
        link.style.cssText = "position:fixed;top:8px;right:16px;z-index:1000;" +
            "padding:6px 14px;border-radius:6px;font-weight:bold;" +
            "font-family:sans-serif;font-size:14px;text-decoration:none;" +
            "color:#ffffff;background:#1f4e9e;border:1px solid #ffffff;";
        document.body.appendChild(link);
    }

    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", addButton);
    } else {
        addButton();
    }
}());
