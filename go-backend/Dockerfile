FROM golang:alpine
RUN mkdir /app
COPY . /app
WORKDIR /app
RUN go mod download
RUN go build -o main .

CMD ["/app/main"]
EXPOSE 5000