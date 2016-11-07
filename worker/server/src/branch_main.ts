/// <reference path="server/defs/node.d.ts" />

var WS_PATH = "ws://199.19.87.92:3001"

var sodium = require('sodium').api
var querystring = require('querystring')

var SERVER_PUBLIC_KEY = new Buffer('3be351d8f39c2216e466564a452fe106dd42487528d3c0ba5b0ed9691106336d', 'hex')
var BRANCH_SECRET_KEY = new Buffer('d6554cf51861d7a56662de154231f23de7d50d50cfdc0e007f0effa3086c5dbf', 'hex')
// branch public key = b167d83e314efd8187218e9c86f41cd28a25c077464ac13c010b41f5cd97047e

var request = require('request')

import assert = require('assert')

import Hashids  = require('hashids')
import WebSocket = require('ws')

var hashids = new Hashids("4320C0E8-0609-45C8-BCC7-D4027AA24445", 8)

function openWebSocket() {
  connect()

  function connect() {
    var reconnectTimeout = null
    var taskTimer = null

    try {
      console.log('Connecting...')

      var ws = new WebSocket(WS_PATH)

      ws.on('open', function open() {
        console.log('Connected.')

        var plainText = new Buffer("0", 'utf8')

        var nonce = new Buffer(24);
        sodium.randombytes_buf(nonce);

        var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

        var msg = Buffer.concat([nonce, cipherText])
        ws.send(msg)

        taskTimer = setTimeout(function() {
          var plainText = new Buffer("0", 'utf8')

          var nonce = new Buffer(24);
          sodium.randombytes_buf(nonce);

          var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

          var msg = Buffer.concat([nonce, cipherText])
          if (ws) ws.send(msg)
        }, 6000);
      })

      ws.on('message', function(data, flags) {
        // console.log("Got a message");

        var nonce = data.slice(0, 24)
        var cipherText = data.slice(24)

        var plainText = sodium.crypto_box_open(cipherText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY).toString('utf8')

        if (plainText === "OK") {
          // Request the next task immediately
          plainText = new Buffer("0", 'utf8')

          nonce = new Buffer(24);
          sodium.randombytes_buf(nonce);

          var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

          var msg = Buffer.concat([nonce, cipherText])
          ws.send(msg)
          return
        }

        var task = null
        try {
          task = JSON.parse(plainText)

        } catch (e) {
          console.log("JSON parse error:", e)
          console.log(plainText)
          return;
        }

        var branchRequestId = task.id;
        if (branchRequestId === undefined) {
          // There is no next task, so set a timer to check again in six seconds.
          // console.log("Setting task timeout")
          taskTimer = setTimeout(function() {
            var plainText = new Buffer("0", 'utf8')

            var nonce = new Buffer(24);
            sodium.randombytes_buf(nonce);

            var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

            var msg = Buffer.concat([nonce, cipherText])
            if (ws) ws.send(msg)
          }, 6000);

          return

        } else if (taskTimer) {
          clearTimeout(taskTimer)
          taskTimer = null
        }

        console.log(task)

        var jsonValue = task.request
        var shortCode = hashids.encode([jsonValue.cardId])

        var url = "http://blue.cards/" + shortCode
        console.log("processing", url)

        if (jsonValue.isUpdate) {
          var putUrl = 'https://api.branch.io/v1/url?url='+querystring.escape(url)
          console.log(putUrl)

          // console.log("Skipping updating Branch.io link")
          // plainText = new Buffer(""+branchRequestId, 'utf8')

          // nonce = new Buffer(24);
          // sodium.randombytes_buf(nonce);

          // var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

          // var msg = Buffer.concat([nonce, cipherText])
          // if (ws) ws.send(msg)

         console.log("Updating Branch.io link")
          request.put(putUrl, {
            json: {
              branch_key: 'key_live_cch9yohTGuEQa5wSvUyBQkoeABa1lxQ1',
              branch_secret: "secret_live_cYTOuEIk1fTFtyWIax5WiwA7RBoWovAU",
              // tags: [ 'tag1', 'tag2' ],
              // channel: 'facebook',
              // feature: 'dashboard',
              // stage: 'new user',
              data: {
                cardId: jsonValue.cardId,
                fullName: jsonValue.fullName,
                location: jsonValue.location ? jsonValue.location : null,
                '$desktop_url': 'http://blue.social/card?id=' + shortCode,
                '$ios_url':     'https://itunes.apple.com/us/app/blue-the-new-way-to-network/id1151689697',
                '$ipad_url':    'https://itunes.apple.com/us/app/blue-the-new-way-to-network/id1151689697',
                '$android_url': 'http://blue.social/card?id=' + shortCode,
              }
            }
          }, function(error, response, result) {
            if (error === null && response.statusCode == 200) {
              try {
                console.log(branchRequestId, "Update Branch.io URL:", url, "for", jsonValue.cardId)
                var plainText = new Buffer(""+branchRequestId, 'utf8')

                var nonce = new Buffer(24);
                sodium.randombytes_buf(nonce);

                var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                var msg = Buffer.concat([nonce, cipherText])
                if (ws) ws.send(msg)

              } catch (e) {
                console.log("WebSocket error", e)

                console.log("Setting task timeout")
                taskTimer = setTimeout(function() {
                  var plainText = new Buffer("0", 'utf8')

                  var nonce = new Buffer(24);
                  sodium.randombytes_buf(nonce);

                  var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                  var msg = Buffer.concat([nonce, cipherText])
                  if (ws) ws.send(msg)
                }, 6000);
              }

            } else {
              console.log("Branch.io error:", result.error)

              // console.log("Setting task timeout")
              taskTimer = setTimeout(function() {
                var plainText = new Buffer("0", 'utf8')

                var nonce = new Buffer(24);
                sodium.randombytes_buf(nonce);

                var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                var msg = Buffer.concat([nonce, cipherText])
                if (ws) ws.send(msg)
              }, 6000);
            }
          })

        } else {
          console.log("Creating Branch.io link")
          request.post('https://api.branch.io/v1/url', {
            json: {
              branch_key: 'key_live_cch9yohTGuEQa5wSvUyBQkoeABa1lxQ1',
               // tags: [ 'tag1', 'tag2' ],
              // channel: 'facebook',
              // feature: 'dashboard',
              // stage: 'new user',
              alias: shortCode,
              data: {
                cardId: jsonValue.cardId,
                fullName: jsonValue.fullName,
                location: jsonValue.location ? jsonValue.location : null,
                '$desktop_url': 'http://blue.social/card?id=' + shortCode,
                '$ios_url':     'https://itunes.apple.com/us/app/blue-the-new-way-to-network/id1151689697',
                '$ipad_url':    'https://itunes.apple.com/us/app/blue-the-new-way-to-network/id1151689697',
                '$android_url': 'http://blue.social/card?id=' + shortCode,
              }
            }
          }, function(error, response, result) {
            if (error === null && response.statusCode == 200) {
              try {
                console.log(branchRequestId, "Branch.io URL:", result.url, "for", jsonValue.cardId)
                var plainText = new Buffer(""+branchRequestId, 'utf8')

                var nonce = new Buffer(24);
                sodium.randombytes_buf(nonce);

                var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                var msg = Buffer.concat([nonce, cipherText])
                if (ws) ws.send(msg)

              } catch (e) {
                console.log("WebSocket error", e)

                // console.log("Setting task timeout")
                taskTimer = setTimeout(function() {
                  var plainText = new Buffer("0", 'utf8')

                  var nonce = new Buffer(24);
                  sodium.randombytes_buf(nonce);

                  var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                  var msg = Buffer.concat([nonce, cipherText])
                  if (ws) ws.send(msg)
                }, 6000);
              }

            } else {
              console.log("Branch.io error:", result.error)

              // console.log("Setting task timeout")
              taskTimer = setTimeout(function() {
                var plainText = new Buffer("0", 'utf8')

                var nonce = new Buffer(24);
                sodium.randombytes_buf(nonce);

                var cipherText = sodium.crypto_box(plainText, nonce, SERVER_PUBLIC_KEY, BRANCH_SECRET_KEY)

                var msg = Buffer.concat([nonce, cipherText])
                if (ws) ws.send(msg)
              }, 6000);
            }
          })
        }
      })

      ws.on('close', function() {
        console.log('Socket closed')

        if (taskTimer) {
          clearTimeout(taskTimer)
          taskTimer = null
        }

        ws = null

        if (reconnectTimeout) {
          clearTimeout(reconnectTimeout)
          reconnectTimeout = null
        }
        reconnectTimeout = setTimeout(function() { openWebSocket() }, 1000)
      })

      ws.on('error', function(err) {
        console.log('Socket error', err)

        if (taskTimer) {
          clearTimeout(taskTimer)
          taskTimer = null
        }

        if (reconnectTimeout) {
          clearTimeout(reconnectTimeout)
          reconnectTimeout = null
        }
        reconnectTimeout = setTimeout(function() { openWebSocket() }, 1000)
      })

    } catch (e) {
      console.log('Caught exception connecting', e)

      ws = null

      if (taskTimer) {
        clearTimeout(taskTimer)
        taskTimer = null
      }

      if (reconnectTimeout) {
        clearTimeout(reconnectTimeout)
        reconnectTimeout = null
      }
      reconnectTimeout = setTimeout(function() { openWebSocket() }, 1000)
    }
  }
}

openWebSocket()
