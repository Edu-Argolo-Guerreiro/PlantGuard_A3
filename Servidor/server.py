import threading
import time

import serial
from flask import Flask, jsonify, request, render_template_string