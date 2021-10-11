package com.example.luxflux

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.BufferedReader
import java.io.PrintWriter
import java.net.ServerSocket


class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        Thread{ server() }.start()
        //server()
        //
    }
    fun server() {
        val server = ServerSocket(3333)
        val client = server.accept()
        //val output = PrintWriter(client.getOutputStream(), true)
        //val input = BufferedReader(InputStreamReader(client.inputStream))
        runOnUiThread { Toast.makeText(applicationContext, "Hey! I'm working", Toast.LENGTH_SHORT).show()}
    }
}