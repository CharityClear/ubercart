<?php
	function uc_charityclear_completed(){
		if($_SESSION['charityclearoutput']){
			$output = $_SESSION['charityclearoutput'];
			$title = $_SESSION['charityclearoutputtitle'];
			$_SESSION['charityclearoutput'] = "";
			$_SESSION['charityclearoutputtitle'] = "";
			drupal_set_title($title);
			return($output);
		}else{
			header('Location: /');
		}
	}

	function uc_charityclear_complete($cart_id = 0) {
		$_SESSION['charityclearoutput'] = "";

		watchdog('charityclear', 'Receiving new order notification for order !order_id.', array('!order_id' => check_plain($_POST['orderRef'])));

		$order = uc_order_load($_POST['orderRef']);

		$formatcheck = strstr($order->order_total, '.');

		if($formatcheck !== false){
			$ordertotal = $order->order_total;
		}else{
			$ordertotal = $order->order_total."00";
		}

		if ( $order === false || (uc_order_status_data( $order->order_status, 'state' ) != 'in_checkout' &&  uc_order_status_data( $order->order_status, 'state' ) != 'payment_received')) {

			if ( uc_order_status_data( $order->order_status, 'state' ) != 'payment_received' ) {
				uc_order_save($order);
			}
			
			$_SESSION['charityclearoutputtitle'] = "Order Failed";
			$_SESSION['charityclearoutput'] = '<br />Unfortunately your payment wasn\'t successful.<br /><a href="/cart/checkout">Please try again.</a>';

		}elseif((!$_POST['amountReceived']) || (!is_numeric($_POST['amountReceived'])) || (str_replace(".","", $ordertotal) != $_POST['amountReceived']) || ($_POST['responseCode'] != 0)){
			if ( uc_order_status_data( $order->order_status, 'state' ) != 'payment_received' ) {
				uc_order_save($order);
			}
			$_SESSION['charityclearoutputtitle'] = "Order Failed";
			$_SESSION['charityclearoutput'] = '<br />Unfortunately your payment wasn\'t successful.<br /><a href="/cart/checkout">Please try again.</a>';


		}else{
			$usefullinfo = "Message: ".$_POST['responseMessage']." - CrossRef: ".$_POST['xref'];
			
			if ( uc_order_status_data( $order->order_status, 'state' ) != 'payment_received' ) {
				uc_payment_enter($order->order_id, 'charityclear', substr($_POST['amountReceived'],0, strlen($_POST['amountReceived'])-2).".".substr($_POST['amountReceived'],strlen($_POST['amountReceived'])-2) , $order->uid, NULL, $usefullinfo);
				uc_order_update_status($order->order_id,"payment_received");

				uc_cart_empty($cart_id);
			}
			$_SESSION['charityclearoutputtitle'] = "Order Completed";
			$_SESSION['charityclearoutput'] = uc_cart_complete_sale($order, variable_get('uc_new_customer_login', FALSE));

		}

		if (isset($_GET['callback'])) {
			http_response_code(200);
			die('callback complete');
		}

		//die(var_dump($order,$_POST));
		$page = "cart/charityclear/completed";

		if (!empty($page)) {
			drupal_goto($page);
		}

		return $output;
	}
