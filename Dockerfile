# Base image (ubuntu:latest for linux/amd64)
FROM ubuntu@sha256:aa6c2c047467afc828e77e306041b7fa4a65734fe3449a54aa9c280822b0d87d

# Set the working directory
WORKDIR /home/dev

# Install packages
RUN apt update -y
RUN apt upgrade -y
RUN apt install -y build-essential curl git language-pack-en nano sudo wget zsh
RUN sh -c "$(curl --location https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin

# Create the dev user
RUN addgroup --gid 1000 dev
RUN adduser --gid 1000 --uid 1000 --disabled-password --gecos '' --home /home/dev --shell /bin/zsh dev
RUN echo 'dev ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN chown dev:dev /home/dev

# Switch to the dev user
USER dev

# Install Oh My Zsh (https://github.com/ohmyzsh/ohmyzsh#unattended-install)
RUN /bin/sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

# Configure Oh My Zsh
RUN sed -i -e 's|ZSH_THEME="robbyrussell"|ZSH_THEME="agnoster"|g' ~/.zshrc

# Run Zsh on start
CMD ["/bin/zsh"]