# Base image (ubuntu:latest for linux/amd64)
FROM ubuntu@sha256:bace9fb0d5923a675c894d5c815da75ffe35e24970166a48a4460a48ae6e0d19

# Create the dev user
RUN addgroup --gid 1000 dev
RUN adduser --gid 1000 --uid 1000 --disabled-password --gecos '' --home /home/dev --shell /bin/zsh dev
RUN echo 'dev ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN chown dev:dev /home/dev -R

# Switch to the dev user
USER dev

# Set the working directory
WORKDIR /home/dev

# Install packages
RUN apt update -y
RUN apt upgrade -y
RUN apt install -y build-essential curl gdb git language-pack-en libeigen3-dev libelf-dev nano sudo unzip wget zsh

# Install Taskfile (See https://taskfile.dev/installation/#install-script)
RUN sh -c "$(curl -fsSL https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin

# Install DynamoRIO (See https://dynamorio.org/page_download.html)
RUN mkdir dynamorio
RUN curl -fsSL https://github.com/DynamoRIO/dynamorio/releases/download/release_9.0.1/DynamoRIO-Linux-9.0.1.tar.gz | tar xzf - -C dynamorio --strip-components 1

# Install Doxybook2 (See https://github.com/matusnovak/doxybook2#Install)
RUN curl -fsSL https://github.com/matusnovak/doxybook2/releases/download/v1.4.0/doxybook2-linux-amd64-v1.4.0.zip -o doxybook2.zip
RUN unzip -j doxybook2.zip bin/doxybook2 -d /usr/local/bin
RUN rm doxybook2.zip

# Install Oh My Zsh (See https://github.com/ohmyzsh/ohmyzsh#unattended-install)
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" -- --unattended
RUN git config --global --add oh-my-zsh.hide-status 1
RUN sed -i -e 's|ZSH_THEME="robbyrussell"|ZSH_THEME="agnoster"|g' ~/.zshrc

# Run Zsh on start
CMD ["/bin/zsh"]