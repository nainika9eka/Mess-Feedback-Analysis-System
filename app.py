from flask import Flask, render_template, request
import subprocess
import os

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/submit', methods=['POST'])
def submit():
    feedback = request.form['feedback'].strip()

    # Save feedback to log file
    with open("food_reviews.txt", "a", encoding="utf-8") as f:
        f.write(feedback + "\n")

    return render_template('index.html', message="✅ Feedback submitted successfully!")

@app.route('/results')
def results():
    try:
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        exe_path = os.path.join(BASE_DIR, "sentiment_analyzer.exe")

        output = subprocess.check_output(
            [exe_path],
            text=True,
            cwd=BASE_DIR,
            stderr=subprocess.STDOUT
        )

        return render_template("result.html", result=output)

    except subprocess.CalledProcessError as e:
        return f"<h3>❌ Error running analysis:<br><pre>{e.output}</pre></h3>"

    except FileNotFoundError:
        return "<h3>❌ sentiment_analyzer.exe not found in project folder.</h3>"

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0")
