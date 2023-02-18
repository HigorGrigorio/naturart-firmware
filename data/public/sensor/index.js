const onsub = async (event) => {
  event.preventDefault();
  let invalid = false;

  document.querySelector(".submit-input").innerHTML = "Enviar";
  document.querySelector("p[name='fetch-message']").innerHTML = "";

  if (!validate("username")) {
    invalid = true;
  }

  if (!validate("password")) {
    invalid = true;
  }

  if (!validate("cpf")) {
    invalid = true;
  }

  if (!validate("serialcode")) {
    invalid = true;
  }

  if (invalid) {
    // dont't receive a response, but show error message.
    httpError(400);
    return;
  }

  document.querySelector(
    ".submit-input"
  ).innerHTML = `<div class="lds-dual-ring"></div>`;

  const username = document.querySelector('input[name="username"]').value;
  const password = document.querySelector('input[name="password"]').value;
  const cpf = document.querySelector('input[name="cpf"]').value;
  const serialcode = document.querySelector('input[name="serialcode"]').value;

  fetch("/", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: `username=${username}&password=${password}&cpf=${cpf}&serialCode=${serialcode}`,
  })
    .then((res) => {
      if (res.status === 200) {
        httpSuccess();
      } else if (res.status === 400) {
        httpError(400);
      } else if (res.status === 422) {
        httpError(422);
      }
    })
    .catch((error) => console.error("Error: " + error));
};
