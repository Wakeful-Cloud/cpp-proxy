# Base image (ubuntu:latest for linux/arm64/v8)
FROM ubuntu@sha256:0f744430d9643a0ec647a4addcac14b1fbb11424be434165c15e2cc7269f70f8

# Set the working directory
WORKDIR /home/dev

# Install packages
RUN apt update -y
RUN apt upgrade -y
RUN apt install -y build-essential cmake curl gdb git language-pack-en libeigen3-dev libelf-dev nano sudo unzip wget zsh

# Install Taskfile (See https://taskfile.dev/installation/#install-script)
RUN sh -c "$(curl -fsSL https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin

# Install DynamoRIO (See https://dynamorio.org/page_download.html)
RUN mkdir dynamorio
RUN curl -fsSL https://github.com/DynamoRIO/dynamorio/releases/download/release_9.0.1/DynamoRIO-AArch64-Linux-9.0.1.tar.gz | tar xzf - -C dynamorio --strip-components 1

# Create the dev user
RUN addgroup --gid 1000 dev
RUN adduser --gid 1000 --uid 1000 --disabled-password --gecos '' --home /home/dev --shell /bin/zsh dev
RUN echo 'dev ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN chown dev:dev /home/dev -R

# Switch to the dev user
USER dev

# Install Oh My Zsh (See https://github.com/ohmyzsh/ohmyzsh#unattended-install)
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended
RUN git config --global --add oh-my-zsh.hide-status 1
RUN sed -i -e 's|ZSH_THEME="robbyrussell"|ZSH_THEME="agnoster"|g' ~/.zshrc

# Run Zsh on start
CMD ["/bin/zsh"]