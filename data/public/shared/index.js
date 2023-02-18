const validate = (name) => {
    const input = document.querySelector(`input[name="${name}"]`);

    if (input.value === '') {
        const error = document.querySelector(`p[name="${name}-error"]`);
        const label = document.querySelector(`label[for="${name}"]`);

        // floating label
        label.classList = ['empty-label'];
        input.classList += ' form-control-empty';
        error.innerHTML = 'O campo não pode ser vazio';
        return false;
    }
    return true;
}

const focusout = (event) => {
    const { target } = event;
    validate(target.name);
}

const focusin = (event) => {
    const { target } = event;

    if (target.value === '') {
        const { name } = target;
        const label = document.querySelector(`label[for="${name}"]`);
        const input = document.querySelector(`input[name="${name}"]`);
        const error = document.querySelector(`p[name="${name}-error"]`);

        label.classList = ['not-empty-label']
        input.classList = ['form-control']
        error.innerHTML = ''
    }
}

const setHttpMessage = (error, isError) => {
    document.querySelector('p[name="fetch-message"]').innerHTML = error;

    if(isError) {
        document.querySelector(
            ".submit-input"
        ).innerHTML = `<div class="x-icon"></div>`;
    } else {
        document.querySelector(
            ".submit-input"
        ).innerHTML = `<div class="check-icon"></div>`;
    }
}

const httpSuccess = () => {
    setHttpMessage('Dados submetidos com sucesso', false);
}

const httpError = (arg) => { 
    if(arg === 400) {
        setHttpMessage('Preencha todos os campos', true);
    } else if(arg === 422) {
        setHttpMessage('Obtivemos problemas ao processar os dados, tente novamente mais tarde.', true);
    }
}