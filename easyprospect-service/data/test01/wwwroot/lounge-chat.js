//"use strict"

/**
 * Creates new chat by connecting via WS to uri
 *
 * @param uri Full URI to connect to
 * @param user_name Identity of user
 *
 * Callbacks:
 *
 * - on_open - Called when connectin is opened.
 * - on_close - Called when connection is closed.
 * - on_error - Called when error hapens. E.g. message is not valid JSON.
 * - on_message - Called when new message arrives.
 */
function LoungeChat(uri, user_name) {
    let ws = new WebSocket(uri)
    let id = 1

    this.send_message = function(method, params) {
        let json = {
            jsonrpc: "2.0",
            method: method,
            id: id,
            params: params,
        }

        ws.send(JSON.stringify(json))
        id++
    }

    this.disconnect = function() {
        ws.close()
    }

    this.on_open = function() {}
    this.on_close = function() {}
    this.on_error = function() {}
    this.on_message = function() {}

    ws.addEventListener('open', function(event) {
        this.send_message("identify", { cid: 1, name: user_name } )
        this.send_message("join", { cid: 2 } )
        this.on_open()
    }.bind(this))

    ws.addEventListener('close', function(event) {
        this.on_close()
    }.bind(this))

    ws.addEventListener('error', function(event) {
        this.on_error(event)
    }.bind(this))

    ws.addEventListener('message', function(event) {
        try {
            this.on_message(JSON.parse(event.data))
        } catch (error) {
            this.on_error(error)
        }
    }.bind(this))
}

//------------------------------------------------------------------------------

function CardText(card)
{
  var s = "AC2C3C4C5C6C7C8C9CTCJCQCKCAH2H3H4H5H6H7H8H9HTHJHQHKHAS2S3S4S5S6S7S8S9STSJSQSKSAD2D3D4D5D6D7D8D9DTDJDQDKD";
  return s.substring(2*(card-1), 2*card);
}

function UpdateSeat(seat, i, e) {
    var s;
    s = '[' + i + '] ';
         if(seat["state"] == "dealer")  s += "(Dealer)";
    else if(seat["state"] == "open")    s += "(Open)";
    else if(seat["state"] == "waiting") s += seat["user"] + " (Waiting)";
    else if(seat["state"] == "playing") s += seat["user"];
    else if(seat["state"] == "leaving") s += seat["user"] + " (Leaving)";
    if(seat["wager"] != undefined) s += " Bet $" + seat["wager"];
    var h = seat["hands"][0];
    for(i = 0; i < h.length; i++) {
        s += ' ' + CardText(h[i]);
    }
    e.textContent = s;
}

function UpdateTable(game, e) {
    e.getElementsByClassName("message")[0].textContent = game.message;
    var seats = e.getElementsByClassName("seat");
    for(var i = 0; i <= 5; i++)
        UpdateSeat(game.seats[i], i, seats[i]);
}
