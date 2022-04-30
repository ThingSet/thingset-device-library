from datetime import datetime
from pathlib import Path
import os

extensions = [
    "breathe",
]

# Project

project = "ThingSet C/C++ Library"
author = "The ThingSet Project Contributors"
year = datetime.now().year
copyright = f"{year} {author}"

version = os.popen("git describe --tags --abbrev=0").read()

# HTML output

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "collapse_navigation": False,
    "style_nav_header_background": "#52595d",
}
html_static_path = ["static"]
html_logo = "static/images/thingset_logo_white.png"
html_context = {
  "display_github": True,
  "github_user": "ThingSet",
  "github_repo": "thingset-device-library",
  "github_version": "main/docs/",
}
html_favicon = "static/images/favicon.ico"

# Breathe

breathe_projects = {"app": "build/doxygen/xml"}
breathe_default_project = "app"
breathe_domain_by_extension = {"h": "c", "c": "c"}
breathe_default_members = ("members", )
