const onsub = async (event) => {
  event.preventDefault();
  let result = false;

  document.querySelector(".submit-input").innerHTML = "Enviar";
  document.querySelector("p[name='fetch-message']").innerHTML = "";

  if (!validate("ssid")) {
    result = true;
  }

  if (!validate("password")) {
    result = true;
  }

  if (result) {
    // dont't receive a response, but show error message.
    httpError(400);
    return;
  }

  document.querySelector(
    ".submit-input"
  ).innerHTML = `<div class="lds-dual-ring"></div>`;

  const ssid = document.querySelector('input[name="ssid"]').value;
  const password = document.querySelector('input[name="password"]').value;

  fetch("/", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: `ssid=${ssid}&password=${password}`,
  })
    .then(adaptHttpFetchHandling())
    .catch((error) => console.error("Error: " + error));
};
